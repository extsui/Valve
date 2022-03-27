﻿#include "ValveMain.h"
#include "Console.h"
#include "RotaryEncoder.h"
#include "main.h"

#include "stm32l0xx_hal_tim.h"

extern UART_HandleTypeDef huart2;
extern TIM_HandleTypeDef htim2;

namespace {

constexpr int RotaryEncoderCount = 4;

RotaryEncoder g_RotaryEncoder[RotaryEncoderCount];

int g_SamplingCount = 0;

struct RotaryEncoderPortPin
{
    PortPin PhaseA;
    PortPin PhaseB;
};

RotaryEncoderPortPin RotaryEncoderPortPinSettings[] = 
{
    // ch1
    {
        PortPin(RotaryEncoder1A_GPIO_Port, RotaryEncoder1A_Pin),
        PortPin(RotaryEncoder1B_GPIO_Port, RotaryEncoder1B_Pin),
    },
    // ch2
    {
        PortPin(RotaryEncoder2A_GPIO_Port, RotaryEncoder2A_Pin),
        PortPin(RotaryEncoder2B_GPIO_Port, RotaryEncoder2B_Pin),
    },
    // ch3
    {
        PortPin(RotaryEncoder3A_GPIO_Port, RotaryEncoder3A_Pin),
        PortPin(RotaryEncoder3B_GPIO_Port, RotaryEncoder3B_Pin),
    },
    // ch4
    {
        PortPin(RotaryEncoder4A_GPIO_Port, RotaryEncoder4A_Pin),
        PortPin(RotaryEncoder4B_GPIO_Port, RotaryEncoder4B_Pin),
    }
};

}

extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    for (auto& rotaryEncoder : g_RotaryEncoder) {
        rotaryEncoder.Sample();
    }
    g_SamplingCount++;
}

void ValveMain()
{
    Console::SetPort(&huart2);
    Console::Log("Valve Start.\n");

    for (int i = 0; i < RotaryEncoderCount; i++) {
        auto setting = &RotaryEncoderPortPinSettings[i];
        g_RotaryEncoder[i].SetPortPin(&setting->PhaseA, &setting->PhaseB);

        // 今回作成した評価基板ではエンコーダ向きが逆のため
        g_RotaryEncoder[i].SetReverse();

        g_RotaryEncoder[i].Sample();
    }

    HAL_TIM_Base_Start_IT(&htim2);

    while (1) {
        if (g_SamplingCount % 50 == 0) {
            Console::Log("%d  % 3d  % 3d  % 3d  % 3d\n",
                g_SamplingCount,
                g_RotaryEncoder[0].GetPosition(),
                g_RotaryEncoder[1].GetPosition(),
                g_RotaryEncoder[2].GetPosition(),
                g_RotaryEncoder[3].GetPosition()
            );
        }
    }
}
