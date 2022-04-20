#include "Utils.h"
#include "I2cSlaveDriver.h"

#include "stm32l0xx_hal_i2c.h"

#include <string.h>

namespace {

volatile int g_RxBufferCount = 0;
volatile int g_TxBufferCount = 0;
volatile uint8_t g_RxBuffer[8];
volatile uint8_t g_TxBuffer[8];

void ClearBuffer(void)
{
    g_RxBufferCount = 0;
    g_TxBufferCount = 0;
    std::memset(const_cast<uint8_t*>(g_RxBuffer), 0, sizeof(g_RxBuffer));
    std::memset(const_cast<uint8_t*>(g_TxBuffer), 0, sizeof(g_TxBuffer));
}

class I2cSlaveDriverImpl
{  
public:
    I2cSlaveDriverImpl()
     : m_Initialized(false)
     , m_pHandle(nullptr)
     , m_OwnAddress(0)
     , m_OnReceivedHandler(nullptr)
    {   
    }

    ~I2cSlaveDriverImpl() = default;
    
    /**
     * @brief I2C スレーブドライバの初期化
     * 
     * @param pHandle I2C ハンドル
     * @param ownAddress 自身のアドレス (スレーブアドレス)
     * @param callback Polling で呼び出される受信コールバック
     */
    void Initialize(I2C_HandleTypeDef *pHandle, uint8_t ownAddress, I2cSlaveDriver::OnReceivedHandler onReceivedHandler) noexcept
    {
        ASSERT(!m_Initialized);
        ASSERT(pHandle != nullptr);
        ASSERT(onReceivedHandler != nullptr);

        m_pHandle = pHandle;
        m_OwnAddress = ownAddress;
        m_OnReceivedHandler = onReceivedHandler;

        // Own Address 1 無効化時のみ設定可能
        pHandle->Instance->OAR1 &= ~I2C_OAR1_OA1EN;
        pHandle->Instance->OAR1 |= (I2C_OAR1_OA1EN | (ownAddress << 1));

        m_Initialized = true;
    }

    /**
     * @brief I2C 受信の開始
     */
    void Listen() noexcept
    {
        ASSERT(m_Initialized);
        HAL_I2C_EnableListen_IT(m_pHandle);
    }

    /**
     * @brief コールバックのラッパー
     */
    int OnReceive(uint8_t *pOutTxData, int *pOutTxSize, const uint8_t *pRxData, int rxSize)
    {
        return m_OnReceivedHandler(pOutTxData, pOutTxSize, pRxData, rxSize);
    }

private:
    bool m_Initialized;
    I2C_HandleTypeDef *m_pHandle;
    uint8_t m_OwnAddress;
    I2cSlaveDriver::OnReceivedHandler m_OnReceivedHandler;
};

I2cSlaveDriverImpl g_Impl;

}

namespace I2cSlaveDriver {

void Initialize(I2C_HandleTypeDef *pHandle, uint8_t ownAddress, OnReceivedHandler onReceivedHandler) noexcept
{
    g_Impl.Initialize(pHandle, ownAddress, onReceivedHandler);
}

void Listen() noexcept
{
    g_Impl.Listen();
}

}

/************************************************************
 *  割り込みハンドラ
 ************************************************************/
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

        if (g_RxBufferCount == 0) {
            return;
        }

        // 割り込みハンドラ上で受信解析する。
        // 重すぎる場合はメインコンテキストに処理を移譲すること。
        // この割り込みハンドラ中はクロックストレッチが有効。
        int txSize = 0;
        auto result = g_Impl.OnReceive(
            const_cast<uint8_t*>(g_TxBuffer),
            &txSize,
            const_cast<uint8_t*>(g_RxBuffer),
            g_RxBufferCount
        );
        
        if ((result == 0) && (txSize >= 1)) {
            ASSERT(txSize < sizeof(g_TxBuffer));
            g_TxBufferCount = txSize;
            HAL_I2C_Slave_Seq_Transmit_IT(hi2c, (uint8_t*)g_TxBuffer, txSize, I2C_NEXT_FRAME);
        }
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
    uint8_t error = HAL_I2C_GetError(hi2c);

    if (error & HAL_I2C_ERROR_BERR) {
        Console::Log("BERR\n");
    }
    if (error & HAL_I2C_ERROR_ARLO) {
        Console::Log("ARLO\n");
    }
    if (error & HAL_I2C_ERROR_AF) {
        // マスタ側は最終バイト受信後に NACK を返してくるので
        // 受信時は必ずこのエラーが発生する
        LED_DEBUG(5);
        //Console::Log("AF\n");
    }
    if (error & HAL_I2C_ERROR_OVR) {
        Console::Log("OVR\n");
    }
    if (error & HAL_I2C_ERROR_DMA) {
        Console::Log("DMA\n");
    }
    if (error & HAL_I2C_ERROR_TIMEOUT) {
        Console::Log("TIMEOUT\n");
    }
    if (error & HAL_I2C_ERROR_SIZE) {
        Console::Log("SIZE\n");
    }
    if (error & HAL_I2C_ERROR_DMA_PARAM) {
        Console::Log("DMA_PARAM\n");
    }
}

extern "C" void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
{
    LED_DEBUG(6);

    {
        if (g_TxBufferCount == 0) {
            int txSize = 0;
            auto result = g_Impl.OnReceive(
                const_cast<uint8_t*>(g_TxBuffer),
                &txSize,
                const_cast<uint8_t*>(g_RxBuffer),
                g_RxBufferCount
            );
            // 送信データは設定されないはず
            ASSERT(txSize == 0);
        }

        #if I2C_DEBUG
            // I2C トランザクション 1 回が完了した後のサマリー表示
            Console::Log("R=%d [ ", g_RxBufferCount);
            for (int i = 0; i < g_RxBufferCount; i++) {
                Console::Log("0x%02x ", g_RxBuffer[i]);
            }
            Console::Log("]\n");
        #endif

        ClearBuffer();
    }

    // I2C 通信で割り込み禁止に落とされているので
    // 通信完了時に割り込み許可に戻す必要がある
    HAL_I2C_EnableListen_IT(hi2c);
}
