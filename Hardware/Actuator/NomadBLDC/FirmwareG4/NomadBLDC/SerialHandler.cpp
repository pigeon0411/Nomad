/*
 * SerialHandler.cpp
 *
 *  Created on: March 19, 2020
 *      Author: Quincy Jones
 *
 * Copyright (c) <2020> <Quincy Jones - quincy@implementedrobotics.com/>
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software
 * is furnished to do so, subject to the following conditions:
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
#include "SerialHandler.h"

// C System Files

// C++ System Files

// Project Includes
#include "main.h"
#include "cmsis_os.h"
#include "shared.h"

// HDLC Handler    
//HDLCHandler hdlc;

// Comms Event Loops

// C interface
extern "C"
{
    void comms_thread_entry()
    {
        // Reset Message Queues.  Not sure if this is actually necessary
        osMessageQueueReset(uart_tx_dma_queue_id);
        osMessageQueueReset(uart_rx_dma_queue_id);

        // 
        for (;;)
        {
            //LL_GPIO_SetOutputPin(GPIOB, GPIO_PIN_10);
            //LL_GPIO_TogglePin(LED_STATUS_GPIO_Port, LED_STATUS_Pin);

            //osDelay(500);
           // LL_GPIO_ResetOutputPin(GPIOB, GPIO_PIN_10);

            //osDelay(100);
        }
        //hdlc.ProcessByte(evt.value.v);
    }
}
