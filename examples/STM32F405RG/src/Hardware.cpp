/*******************************************************************************
 * stm32async: Asynchronous I/O C++ library for STM32
 * Test unit for the development board: STM32F405RGT6
 * *****************************************************************************
 * Copyright (C) 2016-2017 Mikhail Kulesh
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

#include "Hardware.h"
#include "MyApplication.h"

#define USART_DEBUG_MODULE "HRDW: "

/************************************************************************
 * Class Hardware
 ************************************************************************/
Hardware::Hardware ():
    // System, RTC and MCO
    sysClock { HardwareLayout::Interrupt { SysTick_IRQn, 0, 0 } },
    rtc { HardwareLayout::Interrupt { RTC_WKUP_IRQn, 15, 0 } },
    mco { portA, GPIO_PIN_8, RCC_MCO1SOURCE_PLLCLK, RCC_MCODIV_5 },

    // LEDs
    ledGreen { portB, GPIO_PIN_4, Drivers::Led::ConnectionType::CATHODE },
    ledBlue { portB, GPIO_PIN_5, Drivers::Led::ConnectionType::CATHODE },
    ledRed { portB, GPIO_PIN_8, Drivers::Led::ConnectionType::CATHODE },

    // SPI
    spi1 { portA, GPIO_PIN_5, portA, GPIO_PIN_7, portA, UNUSED_PIN, /*remapped=*/ true, NULL,
             HardwareLayout::Interrupt { SPI1_IRQn, 1, 0 },
             HardwareLayout::DmaStream { &dma2, DMA2_Stream5, DMA_CHANNEL_3,
                                         HardwareLayout::Interrupt { DMA2_Stream5_IRQn, 2, 0 } },
             HardwareLayout::DmaStream { &dma2, DMA2_Stream2, DMA_CHANNEL_3,
                                         HardwareLayout::Interrupt { DMA2_Stream2_IRQn, 2, 0 } }
    },
    spi { spi1 },
    ssd { spi, portC, GPIO_PIN_4, true },

    // SD card
    sdio1 {
            portC, /*SDIO_D0*/GPIO_PIN_8 | /*SDIO_D1*/GPIO_PIN_9 | /*SDIO_D2*/GPIO_PIN_10 | /*SDIO_D3*/GPIO_PIN_11 | /*SDIO_CK*/GPIO_PIN_12,
            portD, /*SDIO_CMD*/GPIO_PIN_2,
            HardwareLayout::Interrupt { SDIO_IRQn, 3, 0 },
            HardwareLayout::DmaStream { &dma2, DMA2_Stream6, DMA_CHANNEL_4,
                                        HardwareLayout::Interrupt { DMA2_Stream6_IRQn, 4, 0 } },
            HardwareLayout::DmaStream { &dma2, DMA2_Stream3, DMA_CHANNEL_4,
                                        HardwareLayout::Interrupt { DMA2_Stream3_IRQn, 5, 0 } }
    },
    pinSdPower { portA, GPIO_PIN_15, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP },
    pinSdDetect { portB, GPIO_PIN_3, GPIO_MODE_INPUT, GPIO_PULLUP },
    sdCard { sdio1, pinSdDetect, /*clockDiv=*/ 2 },

    //ESP
    usart2 { portA, GPIO_PIN_2, portA, GPIO_PIN_3, /*remapped=*/ true, NULL,
             HardwareLayout::Interrupt { USART2_IRQn, 6, 0 },
             HardwareLayout::DmaStream { &dma1, DMA1_Stream6, DMA_CHANNEL_4,
                                         HardwareLayout::Interrupt { DMA1_Stream6_IRQn, 7, 0 } },
             HardwareLayout::DmaStream { &dma1, DMA1_Stream5, DMA_CHANNEL_4,
                                         HardwareLayout::Interrupt { DMA1_Stream5_IRQn, 8, 0 } }
    },
    esp { usart2, portA, GPIO_PIN_1, /*sendLed=*/ &ledGreen },
    espSender { esp, /*errorLed=*/ &ledRed },

    // I2S2 Audio
    i2s2 { portB, /*I2S2_CK*/GPIO_PIN_10 | /*I2S2_WS*/GPIO_PIN_12 | /*I2S2_SD*/GPIO_PIN_15,
          /*remapped=*/ true, NULL,
          HardwareLayout::DmaStream { &dma1, DMA1_Stream4, DMA_CHANNEL_0,
                                      HardwareLayout::Interrupt { DMA1_Stream4_IRQn, 9, 0 } },
          HardwareLayout::DmaStream { &dma1, DMA1_Stream3, DMA_CHANNEL_0,
                                      HardwareLayout::Interrupt { DMA1_Stream3_IRQn, 9, 0 } }
    },
    i2s { i2s2 },
    audioDac { i2s,
              /* power    = */ portB, GPIO_PIN_11,
              /* mute     = */ portB, GPIO_PIN_13,
              /* smplFreq = */ portB, GPIO_PIN_14 },
    streamer { sdCard, audioDac },

    // ADC
    adc1 { portA, GPIO_PIN_0, /*remapped=*/ false, NULL,
        HardwareLayout::DmaStream { &dma2, DMA2_Stream0, DMA_CHANNEL_0,
                                    HardwareLayout::Interrupt { DMA2_Stream0_IRQn, 12, 0 } }
    },
    adc { adc1, /*channel=*/ 0, ADC_SAMPLETIME_56CYCLES },

    // USART logger
    usart1 { portB, GPIO_PIN_6, portB, UNUSED_PIN, /*remapped=*/ true, NULL,
             HardwareLayout::Interrupt { USART1_IRQn, 13, 0 },
             HardwareLayout::DmaStream { &dma2, DMA2_Stream7, DMA_CHANNEL_4,
                                         HardwareLayout::Interrupt { DMA2_Stream7_IRQn, 14, 0 } },
             HardwareLayout::DmaStream { &dma2, DMA2_Stream2, DMA_CHANNEL_4,
                                         HardwareLayout::Interrupt { DMA2_Stream2_IRQn, 14, 0 } }
    },
    usartLogger { usart1, 115200 },

    // Test pin
    testPin { portC, GPIO_PIN_7, GPIO_MODE_OUTPUT_PP }
{
    // External oscillators use system pins
    sysClock.setHSE(&portH, GPIO_PIN_0 | GPIO_PIN_1);
    sysClock.setLSE(&portC, GPIO_PIN_14 | GPIO_PIN_15);
}


void Hardware::abort ()
{
    ledRed.turnOn();
    while(1)
    {
        __NOP();
    }
}


void Hardware::initClock (uint32_t pllp)
{
    sysClock.setSysClockSource(RCC_SYSCLKSOURCE_PLLCLK);
    sysClock.getOscParameters().PLL.PLLState = RCC_PLL_ON;
    sysClock.getOscParameters().PLL.PLLSource = RCC_PLLSOURCE_HSE;
    sysClock.getOscParameters().PLL.PLLM = 16;
    sysClock.getOscParameters().PLL.PLLN = 336;
    sysClock.getOscParameters().PLL.PLLP = pllp;
    sysClock.getOscParameters().PLL.PLLQ = 7;
    sysClock.setAHB(RCC_SYSCLK_DIV1, RCC_HCLK_DIV8, RCC_HCLK_DIV8);
    sysClock.setLatency(FLASH_LATENCY_7);
    sysClock.setRTC();
    sysClock.setI2S(192, 2);
    sysClock.start();
}


bool Hardware::start()
{
    // test pin and MCO
    testPin.start();
    testPin.setHigh();
    mco.start();

    // LEDs
    ledGreen.start();
    ledGreen.turnOff();
    ledBlue.start();
    ledBlue.turnOff();
    ledRed.start();
    ledRed.turnOn();

    // Logger
    usartLogger.initInstance();
    USART_DEBUG("--------------------------------------------------------" << UsartLogger::ENDL
                << "Oscillator frequency: " << SystemClock::getInstance()->getHSEFreq()
                << ", MCU frequency: " << SystemClock::getInstance()->getMcuFreq() << UsartLogger::ENDL);

    // For RTC, it is necessary to reset the state since it will not be
    // automatically reset after MCU programming.
    rtc.stop();
    do
    {
        Rtc::Start::Status status = rtc.start(8 * 2047 + 7, RTC_WAKEUPCLOCK_RTCCLK_DIV2);
        USART_DEBUG("RTC status: " << Rtc::Start::asString(status) << " (" << rtc.getHalStatus() << ")" << UsartLogger::ENDL);
    }
    while (rtc.getHalStatus() != HAL_OK);

    // SPI
    DeviceStart::Status devStatus = spi.start(SPI_DIRECTION_1LINE, SPI_BAUDRATEPRESCALER_64, SPI_DATASIZE_8BIT, SPI_PHASE_2EDGE);
    USART_DEBUG("SPI1 status: " << DeviceStart::asString(devStatus) << " (" << spi.getHalStatus() << ")" << UsartLogger::ENDL);
    if (devStatus != DeviceStart::Status::OK)
    {
        return false;
    }

    // SSD
    Drivers::Ssd_74XX595::SegmentsMask sm;
    sm.top = 3;
    sm.rightTop = 5;
    sm.rightBottom = 7;
    sm.bottom = 4;
    sm.leftBottom = 1;
    sm.leftTop = 2;
    sm.center = 6;
    sm.dot = 0;
    ssd.setSegmentsMask(sm);
    ssd.start();

    // ADC
    devStatus = adc.start();
    USART_DEBUG("ADC1 status: " << DeviceStart::asString(devStatus) << " (" << adc.getHalStatus() << ")" << UsartLogger::ENDL);
    if (devStatus != DeviceStart::Status::OK)
    {
        return false;
    }
    adc.setVRef(3.236);

    // SD card
    pinSdDetect.start();
    if (sdCard.isCardInserted() && !initSdCard())
    {
        return false;
    }

    testPin.setLow();
    ledRed.turnOff();
    USART_DEBUG("--------------------------------------------------------" << UsartLogger::ENDL);
    return true;
}


void Hardware::stop ()
{
    // Stop all devices
    sdCard.stop();
    pinSdPower.setHigh();
    pinSdPower.stop();
    pinSdDetect.stop();
    adc.stop();
    ssd.stop();
    spi.stop();
    rtc.stop();
    ledBlue.stop();
    ledRed.stop();
    ledGreen.stop();
    mco.stop();
    testPin.stop();

    // Log resource occupations after all devices (except USART1 for logging, HSE, LSE) are stopped.
    // Desired: one at portB and DMA2 (USART1), one for portC (LSE), one for portH (HSE)
    printResourceOccupation();
    usartLogger.clearInstance();

    sysClock.stop();
}


bool Hardware::initSdCard ()
{
    pinSdPower.start();
    pinSdPower.setLow();
    HAL_Delay(250);
    DeviceStart::Status devStatus = sdCard.start();
    USART_DEBUG("SD card status: " << DeviceStart::asString(devStatus) << " (" << getSdio().getHalStatus() << ")" << UsartLogger::ENDL);
    if (devStatus == DeviceStart::OK)
    {
        getSdio().printInfo();
        devStatus = sdCard.mountFatFs();
        USART_DEBUG("FAT FS card status: " << DeviceStart::asString(devStatus) << " (" << getSdio().getHalStatus() << ")" << UsartLogger::ENDL);
        if (devStatus == DeviceStart::OK)
        {
            sdCard.listFiles();
            return true;
        }
    }
    return false;
}


void Hardware::printResourceOccupation ()
{
    USART_DEBUG("Resource occupations: " << UsartLogger::ENDL
                << UsartLogger::TAB << "portA=" << portA.getObjectsCount() << UsartLogger::ENDL
                << UsartLogger::TAB << "portB=" << portB.getObjectsCount() << UsartLogger::ENDL
                << UsartLogger::TAB << "portC=" << portC.getObjectsCount() << UsartLogger::ENDL
                << UsartLogger::TAB << "portD=" << portD.getObjectsCount() << UsartLogger::ENDL
                << UsartLogger::TAB << "portH=" << portH.getObjectsCount() << UsartLogger::ENDL
                << UsartLogger::TAB << "dma1=" << dma1.getObjectsCount() << UsartLogger::ENDL
                << UsartLogger::TAB << "dma2=" << dma2.getObjectsCount() << UsartLogger::ENDL);
}


/************************************************************************
 * External interrupts
 ************************************************************************/
extern MyApplication * appPtr;

extern "C"
{
    // Errors
    void HardFault_Handler (void)
    {
        appPtr->abort();
    }

    void MemManage_Handler (void)
    {
        appPtr->abort();
    }

    void BusFault_Handler (void)
    {
        appPtr->abort();
    }

    void UsageFault_Handler (void)
    {
        appPtr->abort();
    }

    // System
    void SysTick_Handler (void)
    {
        HAL_IncTick();
        if (Rtc::getInstance() != NULL)
        {
            Rtc::getInstance()->onMilliSecondInterrupt();
        }
    }

    void RTC_WKUP_IRQHandler ()
    {
        if (Rtc::getInstance() != NULL)
        {
            Rtc::getInstance()->processInterrupt();
        }
    }

    void HAL_RTCEx_WakeUpTimerEventCallback (RTC_HandleTypeDef * /*hrtc*/)
    {
        if (Rtc::getInstance() != NULL)
        {
            Rtc::getInstance()->processEventCallback();
            appPtr->scheduleEvent(MyApplication::EventType::SECOND_INTERRUPT);
        }
    }

    // UARTs: uses both USART and DMA interrupts
    void DMA2_Stream7_IRQHandler (void)
    {
        appPtr->getLoggerUsart().processDmaTxInterrupt();
    }

    void USART1_IRQHandler (void)
    {
        appPtr->getLoggerUsart().processInterrupt();
    }

    void DMA1_Stream6_IRQHandler (void)
    {
        appPtr->getEsp().processDmaTxInterrupt();
    }

    void USART2_IRQHandler (void)
    {
        appPtr->getEsp().processInterrupt();
    }

    void HAL_UART_TxCpltCallback (UART_HandleTypeDef * channel)
    {
        if (channel->Instance == USART1)
        {
            appPtr->getLoggerUsart().processCallback(SharedDevice::State::TX_CMPL);
        }
        else if (channel->Instance == USART2)
        {
            appPtr->getEsp().processTxCpltCallback();
        }
    }

    void HAL_UART_RxCpltCallback (UART_HandleTypeDef * channel)
    {
        if (channel->Instance == USART1)
        {
            appPtr->getLoggerUsart().processCallback(SharedDevice::State::RX_CMPL);
        }
        else if (channel->Instance == USART2)
        {
            appPtr->getEsp().processRxCpltCallback();
        }
    }

    void HAL_UART_ErrorCallback (UART_HandleTypeDef * channel)
    {
        if (channel->Instance == USART1)
        {
            appPtr->getLoggerUsart().processCallback(SharedDevice::State::ERROR);
        }
        else if (channel->Instance == USART2)
        {
            appPtr->getEsp().processErrorCallback();
        }
    }

    // SPI
    void DMA2_Stream5_IRQHandler (void)
    {
        appPtr->getSpi().processDmaTxInterrupt();
    }

    void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef * /*channel*/)
    {
        appPtr->getSpi().processCallback(SharedDevice::State::TX_CMPL);
    }

    // SD card
    void DMA2_Stream3_IRQHandler (void)
    {
        appPtr->getSdio().processDmaRxInterrupt();
    }

    void DMA2_Stream6_IRQHandler (void)
    {
        appPtr->getSdio().processDmaTxInterrupt();
    }

    void SDIO_IRQHandler (void)
    {
        appPtr->getSdio().processSdIOInterrupt();
    }

    // I2S
    void DMA1_Stream4_IRQHandler (void)
    {
        appPtr->getI2S().processDmaTxInterrupt();
    }

    void HAL_I2S_TxCpltCallback (I2S_HandleTypeDef * /*channel*/)
    {
        appPtr->getI2S().processCallback(SharedDevice::State::TX_CMPL);
    }

    // ADC
    void DMA2_Stream0_IRQHandler()
    {
        appPtr->getAdc().processDmaRxInterrupt();
    }

    void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef * /*channel*/)
    {
        if (appPtr->getAdc().processConvCpltCallback())
        {
            appPtr->scheduleEvent(MyApplication::EventType::ADC1_READY);
        }
    }
}