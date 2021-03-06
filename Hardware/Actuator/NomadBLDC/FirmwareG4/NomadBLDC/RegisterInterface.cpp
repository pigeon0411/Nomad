/*
 * RegisterInterface.cpp
 *
 *  Created on: March 20, 2020
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
#include "RegisterInterface.h"

// C System Files
#include <cstring>

// C++ System Files
#include <bitset>

// Project Includes
#include <Peripherals/uart.h>
#include <Peripherals/fdcan.h>   
#include <Logger.h>

Register* RegisterInterface::register_map_[kMaxRegisters] = {};
// RegisterInterface::RegisterInterface()
// {    
// }

void RegisterInterface::HandleCommand(FDCANDevice::FDCAN_msg_t &command)
{
    register_command_t *cmd;
    cmd = (register_command_t *)command.data;
    Logger::Instance().Print("Command: %d\r\n", cmd->header.rwx);
    //Logger::Instance().Print("Address: %d : \r\n", cmd->address);

    if(cmd->header.rwx == 0)
    {
        register_reply_t reply;
        // Read
        Logger::Instance().Print("Address: %d \r\n", cmd->header.address);
        Logger::Instance().Print("READ: %d\r\n", register_map_[cmd->header.address]->Get(reply.cmd_data, 0));
    }
    else if(cmd->header.rwx == 1)
    {
        // Write
        Logger::Instance().Print("Write: %d : \r\n", cmd->header.address);
        register_map_[cmd->header.address]->Set((uint8_t *)cmd->cmd_data, 0);
    }
    else if(cmd->header.rwx == 2)
    {
        // Run Function
        Logger::Instance().Print("Execute: %d : \r\n", cmd->header.address);
    }
    
    // std::bitset<2> rwx(cmd->rwx);
    // std::bitset<12> address(cmd->address);
    // std::bitset<8> byte1(cmd->data[0]);
    // std::bitset<8> byte2(cmd->data[1]);   
}

void RegisterInterface::AddRegister(uint16_t address, Register *reg)
{
    //Logger::Instance().Print("Adding: %d: %d\r\n",address, reg->Get<uint16_t>(0));
    register_map_[address] = reg;
}

Register* RegisterInterface::GetRegister(uint16_t address)
{
    return register_map_[address];
}