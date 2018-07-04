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

#ifndef HARDWARE_LAYOUT_DMA1_H_
#define HARDWARE_LAYOUT_DMA1_H_

#include "../HardwareLayout.h"

#ifdef DMA1

namespace HardwareLayout
{

class Dma1 : public HardwareLayout::Dma
{
public:
    explicit Dma1 () :
        Dma { 1, DMA1 }
    {
        // empty
    }
    virtual void onClockEnable () const
    {
        __HAL_RCC_DMA1_CLK_ENABLE();
    }
    virtual void onClockDisable () const
    {
        __HAL_RCC_DMA1_CLK_DISABLE();
    }
};

} // end namespace
#endif
#endif