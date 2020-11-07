/*
 * fdcan.cpp
 *
 *  Created on: November 3, 2020
 *      Author: Quincy Jones
 *
 * Copyright (c) <2020> <Quincy Jones - quincy@implementedrobotics.com/>
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software
 * is furnished to do so, subject to the following conditions
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 */

// Primary Include
#include <Peripherals/fdcan.h>

// C System Files

// C++ System Files
#include <cmath>

// Project Includes
#include <Logger.h>

static FDCANDevice* g_ISR_VTABLE[FDCANDevice::kMaxInterrupts];

// Length to Data Length Code Conversion LUT
static const uint8_t DLC_LUT[] = {
    // 1 to 8 Bytes
    0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8,
    // 9 to 12 Bytes
    0x9, 0x9, 0x9, 0x9,
    // 13 to 16 Bytes
    0xA, 0xA, 0xA, 0xA,
    // 17 to 20 Bytes
    0xB, 0xB, 0xB, 0xB,
    // 21 to 24 Bytes
    0xC, 0xC, 0xC, 0xC,
    // 25 to 32 Bytes
    0xD, 0xD, 0xD, 0xD, 0xD, 0xD, 0xD, 0xD,
    // 33 to 48 Bytes
    0xE, 0xE, 0xE, 0xE, 0xE, 0xE, 0xE, 0xE,
    0xE, 0xE, 0xE, 0xE, 0xE, 0xE, 0xE, 0xE,
    // 49 to 64 Bytes
    0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF,
    0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF};

// Data Length Code to Length Conversion LUT
static const uint8_t LEN_LUT[] = {
    0, 1, 2, 3, 4, 5, 6, 7,
    8, 12, 16, 20, 24, 32, 48, 64};

FDCANDevice::FDCANDevice(FDCAN_GlobalTypeDef *FDCAN, uint32_t node_id, uint32_t bitrate, uint32_t dbitrate) : FDCAN_(FDCAN), enable_interrupt_(false), timings_valid_(false)
{
    config_.id = node_id;          // Lowest Priority Standard ID (2047 max 11-bit)
    config_.bitrate = bitrate;   // 250 kbps
    config_.dbitrate = dbitrate; // 250 kbps
    config_.sp = 0.8f;           // 80%
    config_.data_sp = 0.625f;    // 62.5%
    config_.mode_fd = 1;         // FD CAN ( Default to FD CAN? It is my preference anyways...)

    // For some reason using the stack is not working and communications is faulty. This should not effect or perf.
    hfdcan_ = new FDCAN_HandleTypeDef();
    hfdcan_->Instance = FDCAN;
    CalculateTimings();
}

FDCANDevice::FDCANDevice(FDCAN_GlobalTypeDef *FDCAN, Config_t config) : FDCAN_(FDCAN), config_(config), enable_interrupt_(false), timings_valid_(false)
{
    // For some reason using the stack is not working and communications is faulty.  This should not effect or perf.
    hfdcan_ = new FDCAN_HandleTypeDef();
    hfdcan_->Instance = FDCAN;
    CalculateTimings();
}

// Init FDCAN
bool FDCANDevice::Init()
{
    // No Valid Timings Computed
    if (!timings_valid_)
        return false;

    // Update Handle Defaults.Bit Timing
    hfdcan_->Instance = FDCAN_;
    hfdcan_->Init.ClockDivider = FDCAN_CLOCK_DIV1;
    hfdcan_->Init.FrameFormat = config_.mode_fd == 0 ? FDCAN_FRAME_CLASSIC : config_.bitrate != config_.dbitrate ? FDCAN_FRAME_FD_BRS : FDCAN_FRAME_FD_NO_BRS;
    hfdcan_->Init.Mode = FDCAN_MODE_NORMAL;
    hfdcan_->Init.AutoRetransmission = DISABLE;
    hfdcan_->Init.TransmitPause = DISABLE;
    hfdcan_->Init.ProtocolException = DISABLE;
    hfdcan_->Init.StdFiltersNbr = 1; // TODO: Move to filter support somewhere
    hfdcan_->Init.ExtFiltersNbr = 0;
    hfdcan_->Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;

    // Update Base Header
    tx_header_.IdType = FDCAN_STANDARD_ID; // Support Extended IDs?
    tx_header_.TxFrameType = FDCAN_DATA_FRAME;
    tx_header_.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    tx_header_.BitRateSwitch = config_.bitrate != config_.dbitrate ? FDCAN_BRS_ON : FDCAN_BRS_OFF;
    tx_header_.FDFormat = config_.mode_fd == 0 ? FDCAN_CLASSIC_CAN : FDCAN_FD_CAN;
    tx_header_.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    tx_header_.MessageMarker = 0;

    if (HAL_FDCAN_Init(hfdcan_) != HAL_OK)
    {
        Error_Handler();
    }

    // Setup Filters
    FDCAN_FilterTypeDef sFilterConfig;

    /* Configure standard ID reception filter to Rx FIFO 0 */
    sFilterConfig.IdType = FDCAN_STANDARD_ID;
    sFilterConfig.FilterIndex = 0;
    sFilterConfig.FilterType = FDCAN_FILTER_MASK;
    sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO1;
    sFilterConfig.FilterID1 = config_.id;
    sFilterConfig.FilterID2 = 0x7FF;
    if (HAL_FDCAN_ConfigFilter(hfdcan_, &sFilterConfig) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_FDCAN_ConfigGlobalFilter(hfdcan_, FDCAN_REJECT, FDCAN_REJECT, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE) != HAL_OK)
    {
        Error_Handler();
    }

    if (hfdcan_->Init.DataPrescaler <= 2) // Only valid for Data Prescaler less than 2
    {
        if (HAL_FDCAN_ConfigTxDelayCompensation(hfdcan_, hfdcan_->Init.DataPrescaler * hfdcan_->Init.DataTimeSeg1, 0) != HAL_OK)
        {
            Error_Handler();
        }
        if (HAL_FDCAN_EnableTxDelayCompensation(hfdcan_) != HAL_OK)
        {
            Error_Handler();
        }
    }

    /* Activate Rx FIFO 0 new message notification on both FDCAN instances */
    if (HAL_FDCAN_ActivateNotification(hfdcan_, FDCAN_IT_RX_FIFO1_NEW_MESSAGE, 0) != HAL_OK)
    {
        Logger::Instance().Print("NOT OKAY\r\n");
        Error_Handler();
    }
    Logger::Instance().Print("OKAY\r\n");
    return true;
}

// Enable FDCAN
bool FDCANDevice::Enable()
{
    if (HAL_FDCAN_Start(hfdcan_) != HAL_OK)
    {
        Error_Handler();
        return false;
    }
    return true;
}

// Disable FDCAN
void FDCANDevice::Disable()
{
    if (HAL_FDCAN_Stop(hfdcan_) != HAL_OK)
    {
        Error_Handler();
    }
}

// Send CAN Packet
void FDCANDevice::Send(uint32_t dest_id, uint8_t *data, uint16_t length)
{
    // Invalid Frame Size
    if((config_.mode_fd && length > 64) ||
    (!config_.mode_fd && length > 8))
    {
        return;
    }

    tx_header_.Identifier = dest_id;
    tx_header_.DataLength = DLC_LUT[length] << 16;

    if (HAL_FDCAN_AddMessageToTxFifoQ(hfdcan_, &tx_header_, data) != HAL_OK)
    {
      Error_Handler();
    }
}

// Receive CAN Packet
bool FDCANDevice::Receive(uint8_t *data, uint16_t &length)
{
    if (HAL_FDCAN_GetRxMessage(hfdcan_, FDCAN_RX_FIFO0, &rx_header_, can_rx_buffer_) != HAL_OK)
    {
        data = nullptr;
        return false;
    }

    // Convert Data Length Code
    length = LEN_LUT[rx_header_.DataLength >> 16];

    // Update Data Pointer
    memcpy(data, can_rx_buffer_, length); 
    return true;
}

void FDCANDevice::EnableIT()
{
    // Find IRQ Number
    if(FDCAN_ == FDCAN1)
    {
        IRQn_ = FDCAN1_IT0_IRQn;
    }
    else if(FDCAN_ == FDCAN2)
    {
        IRQn_ = FDCAN2_IT0_IRQn;
    }
    else if(FDCAN_ == FDCAN3)
    {
        IRQn_ = FDCAN3_IT0_IRQn;
    }
    else // Invalid
    {
        return;
    }

    // Make sure IRQ is enabled
    // // TODO: Priority...
    // NVIC_SetPriority(IRQn_, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),1, 0));
    // NVIC_EnableIRQ(IRQn_);

    // Update ISR Table
    g_ISR_VTABLE[IRQn_] = this;

    // /* Activate Rx FIFO 0 new message notification on both FDCAN instances */
    // if (HAL_FDCAN_ActivateNotification(hfdcan_, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
    // {
    //     Error_Handler();
    // }

    Logger::Instance().Print("GOT: %x to %x", this, g_ISR_VTABLE[FDCAN3_IT0_IRQn] );
    // Set Interrupt Enabled Bookeeping flag
    enable_interrupt_ = true;

    // Not Sure Why Dynamic Does not work
    // __disable_irq();
    // NVIC_SetVector(IRQn, (uint32_t)&IRQ);
    // __enable_irq();
}

// Interrupts
// void FDCAN1_IT0_IRQHandler(void)
// {
//     g_ISR_VTABLE[FDCAN1_IT0_IRQn]->ISR();    
// }

// void FDCAN1_IT1_IRQHandler(void)
// {
//     g_ISR_VTABLE[FDCAN1_IT1_IRQn]->ISR();    
// }

// void FDCAN2_IT0_IRQHandler(void)
// {
//     g_ISR_VTABLE[FDCAN2_IT0_IRQn]->ISR();    
// }

// void FDCAN2_IT1_IRQHandler(void)
// {
//     g_ISR_VTABLE[FDCAN2_IT1_IRQn]->ISR();    
// }

void FDCAN3_IT0_IRQHandler(void)
{
    Logger::Instance().Print("IT\r\n");
    g_ISR_VTABLE[FDCAN3_IT0_IRQn]->ISR();    
}

void FDCAN3_IT1_IRQHandler(void)
{
    Logger::Instance().Print("IT\r\n");
    g_ISR_VTABLE[FDCAN3_IT1_IRQn]->ISR();    
}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    uint8_t buffer[64];
    FDCAN_RxHeaderTypeDef rx_header_;
  if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != 0)
  {
    /* Retrieve Rx messages from RX FIFO0 */
    if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_header_, buffer) != HAL_OK)
    {
      Error_Handler();
    }
    Logger::Instance().Print("Hello!!!\r\n");
  }
}

// Helpers
bool FDCANDevice::CalculateTimings()
{
    // Get CAN Clock
    float f_clkcan = HAL_RCC_GetPCLK1Freq();
    
    // Bit time for desired rate
    float bit_time = 1.0f / config_.bitrate;

    int t_sync = 1; // 1 TQ for Sync

    // TODO: Support Propagation Delays?
    // uint_t t_prop = 250; // propagation delay ns
    uint16_t t_seg1 = 0;
    uint16_t t_seg2 = 0;

    // Reset Timings Flag
    timings_valid_ = false;

    // Compute Nominal Bitrate Timings
    uint16_t prescaler = 0;
    for(prescaler = 1; prescaler < kMaxNominalPrescaler; prescaler++)
    {
        // How many Time Quanta in this prescaler?
        float tq = static_cast<float>(prescaler) / f_clkcan;

        // Number of Time Quanta per bit
        float num_tq = bit_time / tq;

        // Check Whole Time Quanta
        if(std::fmod(num_tq, 1) != 0)
            continue;

        t_seg1 = num_tq * config_.sp - t_sync;
        t_seg2 = num_tq - t_seg1 - t_sync;

        // Verify Time Segment Range 1
        if(t_seg1 < kMinNominalTimeSeg || t_seg1 > kMaxNominalTimeSeg1)
        {
            // Try Next Prescaler
            continue;
        }

        // Verify Time Segment Range 2 
        if(t_seg2 < kMinNominalTimeSeg || t_seg2 > kMaxNominalTimeSeg2)
        {
            // Try Next Prescaler
            continue;
        }

        // Update FDCAN Init Struct for HAL
        hfdcan_->Init.NominalPrescaler = prescaler;
        hfdcan_->Init.NominalTimeSeg1 = t_seg1;
        hfdcan_->Init.NominalTimeSeg2 = t_seg2;
        hfdcan_->Init.NominalSyncJumpWidth = t_seg2;
        timings_valid_ = true;
        break;
    }

    // Check Valid Nominal Bitrate
    if(!timings_valid_)
        return false;

    // Bit time for desired data rate
    bit_time = 1.0f / config_.dbitrate;

    // TODO: Support Propagation Delays?
    t_seg1 = 0;
    t_seg2 = 0;

    // Update Valid Return
    timings_valid_ = false;

    // Compute Data Bitrate Timings
    prescaler = 0;
    for(prescaler = 1; prescaler < kMaxDataPrescaler; prescaler++)
    {
        // How many Time Quanta in this prescaler?
        float tq = static_cast<float>(prescaler) / f_clkcan;

        // Number of Time Quanta per bit
        float num_tq = bit_time / tq;

        // Check Whole Time Quanta
        if(std::fmod(num_tq, 1) != 0)
            continue;

        t_seg1 = num_tq * config_.data_sp - t_sync;
        t_seg2 = num_tq - t_seg1 - t_sync;

        // Verify Time Segment Range 1
        if(t_seg1 < kMinDataTimeSeg || t_seg1 > kMaxDataTimeSeg1)
        {
            // Try Next Prescaler
            continue;
        }

        // Verify Time Segment Range 2 
        if(t_seg2 < kMinDataTimeSeg || t_seg2 > kMaxDataTimeSeg2)
        {
            // Try Next Prescaler
            continue;
        }

        // Update FDCAN Init Struct for HAL
        hfdcan_->Init.DataPrescaler = prescaler;
        hfdcan_->Init.DataTimeSeg1 = t_seg1;
        hfdcan_->Init.DataTimeSeg2 = t_seg2;
        hfdcan_->Init.DataSyncJumpWidth = t_seg2;
        timings_valid_ = true;
        break;
    }
    return timings_valid_;
}

