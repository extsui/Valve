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

// TODO: Utils 系 (共通ファイルにまとめるべきかも)

static inline void ABORT_NO_MESSAGE() { while (1); }

#define LOG(format, ...) Console::Log(format, __VA_ARGS__)

#define ABORT() \
    LOG("Abort: file %s, line %d\n", __FILE__, __LINE__); \
    ABORT_NO_MESSAGE()

#define __ASSERT(expr, file, line)                  \
	LOG("Assertion failed: %s, file %s, line %d\n", \
		expr, file, line),                          \
	ABORT_NO_MESSAGE()

#define ASSERT(expr)                              \
    ((expr) ? ((void)0) :                         \
    (void)(__ASSERT(#expr, __FILE__, __LINE__)))

// TODO: デバッグ用に static を付けてない
volatile int g_RxBufferCount = 0;
volatile int g_TxBufferCount = 0;
volatile uint8_t g_RxBuffer[8];
volatile uint8_t g_TxBuffer[8];

// 送信準備出来たか
volatile bool g_IsReadyTx = false;

// I2C トランザクションが完了した場合に ON
volatile bool g_TransactionCompleted = false;

void ClearBuffer(void)
{
    g_RxBufferCount = 0;
    g_TxBufferCount = 0;
    std::memset(const_cast<uint8_t*>(g_RxBuffer), 0, sizeof(g_RxBuffer));
    std::memset(const_cast<uint8_t*>(g_TxBuffer), 0, sizeof(g_TxBuffer));
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
int OnReceivedHandler(uint8_t *pOutTxData, int *pOutTxSize, const uint8_t *pRxData, int rxSize)
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
            if (pRxData[i + 1] == 0x01) {
                g_RotaryEncoder[i].SetReverse();
            }
        }
        return 0;

    default:
        return -1;
    }
}

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
        // それまではクロックストレッチが効く
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

            // TORIEAZU: 受信解析が遅れた場合、次の受信が始まってしまうと受信バッファが上書きされるが未対処。

            int txSize = 0;
            auto result = OnReceivedHandler(
                const_cast<uint8_t*>(g_TxBuffer),
                &txSize,
                const_cast<uint8_t*>(g_RxBuffer),
                g_RxBufferCount
            );
            
            if ((result == 0) && (txSize >= 1)) {
                ASSERT(txSize < sizeof(g_TxBuffer));
                g_TxBufferCount = txSize;
                HAL_I2C_Slave_Seq_Transmit_IT(&hi2c1, (uint8_t*)g_TxBuffer, txSize, I2C_NEXT_FRAME);
            }
        }

        // I2C トランザクション 1 回が完了した後のサマリー表示
        if (g_TransactionCompleted) {
            g_TransactionCompleted = false;

            if (g_TxBufferCount == 0) {
                int txSize = 0;
                auto result = OnReceivedHandler(
                    const_cast<uint8_t*>(g_TxBuffer),
                    &txSize,
                    const_cast<uint8_t*>(g_RxBuffer),
                    g_RxBufferCount
                );
                // 送信データは設定されないはず
                ASSERT(txSize == 0);
            }

            Console::Log("R=%d [ ", g_RxBufferCount);
            for (int i = 0; i < g_RxBufferCount; i++) {
                Console::Log("0x%02x ", g_RxBuffer[i]);
            }
            Console::Log("]\n");

            ClearBuffer();
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
