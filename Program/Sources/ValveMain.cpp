#include "ValveMain.h"
#include "Console.h"
#include "RotaryEncoder.h"
#include "main.h"

#include <string.h>

#include "stm32l0xx_hal_tim.h"
#include "stm32l0xx_hal_i2c.h"

extern UART_HandleTypeDef huart2;
extern TIM_HandleTypeDef htim2;
extern I2C_HandleTypeDef hi2c1;

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

static inline void LED_DEBUG(int count)
{
    for (int i = 0; i < count; i++) {
        HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
        HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    }
}

}

extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    for (auto& rotaryEncoder : g_RotaryEncoder) {
        rotaryEncoder.Sample();
    }
    g_SamplingCount++;
}

// TODO: デバッグ用に static を付けてない
volatile int g_RxBufferCount = 0;
volatile int g_TxBufferCount = 0;
volatile uint8_t g_RxBuffer[8];
volatile uint8_t g_TxBuffer[8];

// 送信準備出来たか
volatile bool g_IsReadyTx = false;

// I2C トランザクションが完了した場合に ON
volatile bool g_TransactionCompleted = false;

// アドレス一致時点で呼び出される
extern "C" void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
{
    UNUSED(AddrMatchCode);

    // 対象チャンネルでない場合は終了
    if (hi2c->Instance != I2C1) {
        return;
    }

    if (TransferDirection == I2C_DIRECTION_TRANSMIT) {
        // (1) 受信開始のタイミング (Master --> Slave)
        LED_DEBUG(1);

        g_RxBufferCount = 0;
        std::memset(const_cast<uint8_t*>(g_RxBuffer), 0, sizeof(g_RxBuffer));
        // 何 byte 受信するか不明のため 1 byte ずつ割り込みを発生させる (応答は ACK 固定)
        HAL_I2C_Slave_Seq_Receive_IT(hi2c, (uint8_t*)g_RxBuffer, 1, I2C_NEXT_FRAME);

    } else {
        // (3) 送信開始のタイミング (Master <-- Slave)
        // この時点で受信バッファは確定する
        LED_DEBUG(3);

        // 送信開始はメインコンテキストから行う
        // それまではクロックストレッチが効くハズ (要確認)
        g_IsReadyTx = true;
    }
}

// データ受信が完了したタイミングで呼び出される (Master to Slave)
extern "C" void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)      
{
    // (2) 1byte 受信完了タイミング
    LED_DEBUG(2);

    // バッファオーバーラン対策
    // 溢れたら最終バイトを上書きし続ける挙動とする
    if (g_RxBufferCount < sizeof(g_RxBuffer) - 1) {
        g_RxBufferCount++;
    }
    // 何 byte 受信するか不明のため 1 byte ずつ割り込みを発生させる (応答は ACK 固定)
    HAL_I2C_Slave_Seq_Receive_IT(hi2c, (uint8_t*)&g_RxBuffer[g_RxBufferCount], 1, I2C_NEXT_FRAME);
}

// データ送信が完了したタイミングで呼び出される (Slave to Master)
extern "C" void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    // (4) 1byte 送信完了タイミング
    LED_DEBUG(4);

    g_TxBufferCount++;
    HAL_I2C_Slave_Seq_Transmit_IT(hi2c, (uint8_t*)&g_TxBuffer[g_TxBufferCount], 1, I2C_NEXT_FRAME);
}

extern "C" void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    //Console::Log("(E)\n");
    LED_DEBUG(5);

    uint8_t error = HAL_I2C_GetError(hi2c);

    if (error & HAL_I2C_ERROR_BERR) {
        Console::Log("  BERR\n");
    }
    if (error & HAL_I2C_ERROR_ARLO) {
        Console::Log("  ARLO\n");
    }
    if (error & HAL_I2C_ERROR_AF) {
        //Console::Log("  AF\n");
        // マスタ側は最終バイト受信後に NACK を返してくるので受信時は必ずこのエラーが発生する
    }
    if (error & HAL_I2C_ERROR_OVR) {
        Console::Log("  OVR\n");
    }
    if (error & HAL_I2C_ERROR_DMA) {
        Console::Log("  DMA\n");
    }
    if (error & HAL_I2C_ERROR_TIMEOUT) {
        Console::Log("  TIMEOUT\n");
    }
    if (error & HAL_I2C_ERROR_SIZE) {
        Console::Log("  SIZE\n");
    }
    if (error & HAL_I2C_ERROR_DMA_PARAM) {
        Console::Log("  DMA_PARAM\n");
    }
}

extern "C" void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
{
    LED_DEBUG(6);

    // マスタからの受信のみの場合は受信継続中になっているので一旦受信を止める必要がある
    volatile HAL_I2C_StateTypeDef state = HAL_I2C_GetState(hi2c);
    if (state != HAL_I2C_STATE_READY) {
        // ここに来ない... (1)->(2)->(2)->(L)->(4) の順で来て、以降の I2C が出来なくなる
        HAL_I2C_Init(hi2c);
        Console::Log("I2C トランザクション異常終了\n");
    }

    g_TransactionCompleted = true;

    // I2C 通信で割り込み禁止に落とされているので
    // 通信完了時に割り込み許可に戻す必要がある
    HAL_I2C_EnableListen_IT(&hi2c1);
}

void ValveMain()
{
    Console::SetPort(&huart2);
    Console::Log("Valve Start.\n");

    HAL_I2C_EnableListen_IT(&hi2c1);

    for (int i = 0; i < RotaryEncoderCount; i++) {
        auto setting = &RotaryEncoderPortPinSettings[i];
        g_RotaryEncoder[i].SetPortPin(&setting->PhaseA, &setting->PhaseB);

        // 今回作成した評価基板ではエンコーダ向きが逆のため
        g_RotaryEncoder[i].SetReverse();

        g_RotaryEncoder[i].Sample();
    }

    HAL_TIM_Base_Start_IT(&htim2);

    while (1) {

        // I2C 送信 (Master <-- Slave)
        if (g_IsReadyTx) {
            g_IsReadyTx = false;
            
            // 受信データがあったら解析
            if (g_RxBufferCount == 0) {
                Console::Log("受信データ無しで送信要求は異常\n");
                continue;
            }

            // TODO: 受信のみの場合の解析と処理を合わせたい
            // TORIAEZU: 受信後送信のケースのみ実装しておく
            g_TxBufferCount = 0;
            g_TxBuffer[0] = 0x11;
            g_TxBuffer[1] = 0x22;
            g_TxBuffer[2] = 0x33;
            g_TxBuffer[3] = 0x44;
            HAL_I2C_Slave_Seq_Transmit_IT(&hi2c1, (uint8_t*)g_TxBuffer, 1, I2C_NEXT_FRAME);
        }

        // I2C トランザクション 1 回が完了した後のサマリー表示
        if (g_TransactionCompleted) {
            g_TransactionCompleted = false;

            Console::Log("R=%d [ ", g_RxBufferCount);
            for (int i = 0; i < g_RxBufferCount; i++) {
                Console::Log("0x%02x ", g_RxBuffer[i]);
            }
            Console::Log("]\n");
        }

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
