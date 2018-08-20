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

#ifndef HARDWARE_LAYOUT_DAC_H_
#define HARDWARE_LAYOUT_DAC_H_

#include "HardwareLayout.h"

#ifdef HAL_DAC_MODULE_ENABLED

namespace Stm32async
{
namespace HardwareLayout
{

/**
 * @brief Parameters of Digital-to-Analog converter.
 */
class Dac : public HalDevice
{
    DECLARE_INSTANCE(DAC_TypeDef)

public:

    /**
     * @brief Pin related to this DAC
     */
    Pins pin;

    /**
     * @brief AFIO module. Set to NULL in case it's not required (for example, on STM32F4 MCU)
     */
    Afio * afio;

    explicit Dac (size_t _id, DAC_TypeDef *_instance, Port & _port, uint32_t _pin) :
        HalDevice { _id, false },
        instance { _instance },
        pin { _port, _pin },
        afio { NULL }
    {
        // empty
    }
};

} // end of namespace HardwareLayout
} // end of namespace Stm32async

#endif
#endif
