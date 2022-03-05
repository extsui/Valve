#include "I2cSlaveDriver.h"

//#ifdef I2C_SLAVE_DRIVER_DEBUG
#include "Console.h"
//#endif

#include <string.h>

namespace {

// TORIAEZU: アドレスは適当
I2cSlaveDriver m_Impl(0x20);

}

/**
 * スレーブ受信割り込みロック
 */
class I2cSlaveLock
{
private:
    I2C_HandleTypeDef *m_pI2cHandle;

public:
    // スレーブ受信割り込み禁止
    I2cSlaveLock(I2C_HandleTypeDef *pI2cHandle)
        : m_pI2cHandle(pI2cHandle)
    {
        // 受信完了割り込み、アドレス一致割り込み、STOP 検出割り込み、エラー割り込み
        m_pI2cHandle->Instance->CR1 &= ~(I2C_CR1_RXIE | I2C_CR1_ADDRIE | I2C_CR1_STOPIE | I2C_CR1_ERRIE);
    }

    // スレーブ受信割り込み許可
    ~I2cSlaveLock()
    {
        m_pI2cHandle->Instance->CR1 |=  (I2C_CR1_RXIE | I2C_CR1_ADDRIE | I2C_CR1_STOPIE | I2C_CR1_ERRIE);
    }
};

I2cSlaveDriver::I2cSlaveDriver(uint8_t ownAddress)
    : m_OwnAddress(ownAddress)
{
    //ASSERT((1 <= ownAddress) && (ownAddress <= 127));
    //ASSERT(g_Instance == NULL);
    m_ReceiveCount = 0;

    // --------------------------------------------------
    // I2C1 BSP の初期化 (HAL_I2C_MspInit() から流用)
    // --------------------------------------------------
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    // I2C1 GPIO Configuration
    // PA9     ------> I2C1_SCL
    // PA10     ------> I2C1_SDA
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_I2C1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Peripheral clock enable
    __HAL_RCC_I2C1_CLK_ENABLE();

    // I2C1 interrupt Init
    HAL_NVIC_SetPriority(I2C1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(I2C1_IRQn);
    HAL_NVIC_SetPriority(I2C1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(I2C1_IRQn);

    // --------------------------------------------------
    // I2C1 ペリフェラルの初期化
    // --------------------------------------------------
    I2C_TypeDef *i2c = m_pI2cHandle->Instance;

    // ペリフェラルの無効化 (各種設定は無効化中のみ設定可能)
    i2c->CR1     &= ~I2C_CR1_PE;
    i2c->CR1      = 0;
    i2c->CR2      = 0;
    i2c->OAR1     = 0;
    i2c->OAR2     = 0;
    i2c->TIMINGR  = 0;
    i2c->TIMEOUTR = 0;
    i2c->ICR      = 0;

    // 受信完了割り込み、アドレス一致割り込み、STOP 検出割り込み、エラー割り込み許可
    i2c->CR1 |=  (I2C_CR1_RXIE | I2C_CR1_ADDRIE | I2C_CR1_STOPIE | I2C_CR1_ERRIE);

    // クロックストレッチ無し
    // 参考: CPU 16MHz I2C 100kHz 受信、多重割り込み無しでストレッチ発生せずロス無し。
    // 何らかの不具合が発生して SCL をストレッチされる方がリスキーなのでストレッチ無とする。
    i2c->CR1  |=  I2C_CR1_NOSTRETCH;

    // 受信時に ACK 応答
    i2c->CR2  &= ~I2C_CR2_NACK;

    // Own Address 1 無効化時のみ設定可能
    i2c->OAR1 &= ~I2C_OAR1_OA1EN;
    i2c->OAR1 |= (I2C_OAR1_OA1EN | (m_OwnAddress << 1));

    // ペリフェラルの有効化
    i2c->CR1  |=  I2C_CR1_PE;
}

I2cSlaveDriver::~I2cSlaveDriver()
{
    // 破棄禁止
    //ASSERT(0);
}

/************************************************************
 *  割り込みハンドラ
 ************************************************************/
void I2cSlaveDriver::EventHandler() noexcept
{
    I2C_TypeDef *i2c = m_pI2cHandle->Instance;

    // アドレス一致
    if (i2c->ISR & I2C_ISR_ADDR) {
        i2c->ICR |= I2C_ICR_ADDRCF;
        
        // TODO:
    }

    // 受信完了
    if (i2c->ISR & I2C_ISR_RXNE) {
        // バッファオーバーランは警告を表示した上で受信継続
        // バッファオーバーラン時も RXNE を落とすためにレジスタリードは必要
        uint8_t data = i2c->RXDR;
        //Console::Log("[i2c] Frame.Buffer Overrun!\n");

        // TODO:
    }

    // STOP 検出
    if (i2c->ISR & I2C_ISR_STOPF) {
        i2c->ICR |= I2C_ICR_STOPCF;
        m_ReceiveCount++;

        // TODO:
    }
}

void I2cSlaveDriver::ErrorHandler() noexcept
{
    I2C_TypeDef *i2c = m_pI2cHandle->Instance;

    // バスエラー
    if (i2c->ISR & I2C_ISR_BERR) {
        i2c->ICR |= I2C_ICR_BERRCF;
        Console::Log("[i2c] Bus Error!\n");
        
        // TODO:
    }

    // オーバーラン/アンダーラン
    if (i2c->ISR & I2C_ISR_OVR) {
        i2c->ICR |= I2C_ICR_OVRCF;
        Console::Log("[i2c] Overrun!\n");

        // TODO:
    }
}

void aaa(void)
{
    Console::Log("I2cSlaveDriver Instance Size = %d\n", sizeof(m_Impl));
    Console::Log("I2C_HandleTypeDef Size       = %d\n", sizeof(I2C_HandleTypeDef));
    Console::Log("I2C_InitTypeDef Size         = %d\n", sizeof(I2C_InitTypeDef));
}

extern "C" void I2C1_IRQHandler(void)
{
    m_Impl.EventHandler();
}
