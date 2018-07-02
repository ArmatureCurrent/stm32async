/*******************************************************************************
 * stm32async: Asynchronous I/O C++ library for STM32
 * *****************************************************************************
 * Copyright (C) 2018 Mikhail Kulesh, Denis Makarov
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#include "IOPort.h"

using namespace Stm32async;

/************************************************************************
 * Class IOPort
 ************************************************************************/

IOPort::IOPort (const HardwareLayout::Port & _port, uint32_t pins, uint32_t mode, uint32_t pull,
                uint32_t speed, bool callStart) :
    port { _port }
{
    parameters.Pin = pins;
    parameters.Mode = mode;
    parameters.Pull = pull;
    parameters.Speed = speed;
    if (callStart)
    {
        start();
    }
}

void IOPort::start ()
{
    port.enableClock();
    HAL_GPIO_Init(port.instance, &parameters);
}

void IOPort::stop ()
{
    HAL_GPIO_DeInit(port.instance, parameters.Pin);
    port.disableClock();
}