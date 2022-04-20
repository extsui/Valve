#include "Utils.h"
#include "ValveMain.h"
#include "RotaryEncoder.h"
#include "I2cSlaveDriver.h"
#include "main.h"

#include <string.h>

#include "stm32l0xx_hal_i2c.h"
#include "stm32l0xx_hal_cortex.h" // HAL_SYSTICK_Callback

extern UART_HandleTypeDef huart2;
extern I2C_HandleTypeDef hi2c1;

namespace {

bool g_IsEncoderInitialized = false;

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

extern "C" void HAL_SYSTICK_Callback(void)
{
    if (!g_IsEncoderInitialized) {
        return;
    }

    for (auto& rotaryEncoder : g_RotaryEncoder) {
        rotaryEncoder.Sample();
    }
    g_SamplingCount++;
}

/**
 * @brief I2C スレーブ受信完了ハンドラ
 * 
 * - スレーブ受信のみの場合:
 *   - StopCondition 後に呼び出される
 * - スレーブ受信後送信の場合:
 *   - RestartCondition 後のアドレス一致後に呼び出される
 *   - この場合 StopCondition 後には呼び出されない
 * 
 * @param pOutTxData マスタに送信するデータ
 * @param pOutTxSize マスタに送信するサイズ (送信しない場合は 0 を設定すること)
 * @param pRxData マスタから受信したデータ
 * @param rxSize マスタから受信したサイズ
 * @return int 
 */
int I2cReceivedHandler(uint8_t *pOutTxData, int *pOutTxSize, const uint8_t *pRxData, int rxSize)
{
    ASSERT(pOutTxData != nullptr);
    ASSERT(pOutTxSize != nullptr);
    ASSERT(rxSize >= 1);

    *pOutTxSize = 0;

    // 受信ケース
    // TORIAEZU: マスタに送信する途中で NACK を返されて強制中断した異常系は未対応
    // 
    // - WhoAmI
    //   - M->S: [ 0x80 ]
    //   - M<-S: [ 0xAA ]
    // - GetEncoderValue
    //   - M->S: [ 0x00 ]
    //   - M<-S: [ 0xhh 0xhh 0xhh 0xhh ]
    // - SetReverse
    //   - M->S: [ 0x10 0xbb 0xbb 0xbb 0xbb ]
    //   - M<-S: []

    uint8_t operationId = pRxData[0];
    switch (operationId) {
    case 0x80:
        if (rxSize != 1) {
            return -2;
        }
        *pOutTxSize = 1;
        pOutTxData[0] = 0xAA;
        return 0;

    case 0x00:
        if (rxSize != 1) {
            return -2;
        }
        *pOutTxSize = 4;
        for (int i = 0; i < RotaryEncoderCount; i++) {
            pOutTxData[i] = static_cast<uint8_t>(g_RotaryEncoder[i].GetPosition());
        }
        return 0;

    case 0x10:
        if (rxSize != 5) {
            return -2;
        }
        *pOutTxSize = 0;
        for (int i = 0; i < RotaryEncoderCount; i++) {
            switch (pRxData[i + 1]) {
            case 0x00:
                g_RotaryEncoder[i].SetReverse(false);
                break;
            case 0x01:
                g_RotaryEncoder[i].SetReverse(true);
                break;
            default:
                return -2;
            }
        }
        return 0;

    default:
        return -1;
    }
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
        // 初期サンプリング
        g_RotaryEncoder[i].Sample();
    }
    g_IsEncoderInitialized = true;

    // I2C 受信開始
    I2cSlaveDriver::Initialize(&hi2c1, 0x40, I2cReceivedHandler);
    I2cSlaveDriver::Listen();

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
