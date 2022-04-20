#ifndef I2C_SLAVE_DRIVER_H
#define I2C_SLAVE_DRIVER_H

#include <stdint.h>
#include <string.h>
#include <stm32l0xx.h>
#include "stm32l0xx_hal_i2c.h"

typedef int (*OnReceivedHandler)(uint8_t *pOutTxData, int *pOutTxSize, const uint8_t *pRxData, int rxSize);

/**
 * @brief I2C スレーブドライバ
 * 
 * - 送受信対応
 * - 正常系のみ
 * - エラーハンドリングに TBD 多数
 */
class I2cSlaveDriver
{  
public:
    I2cSlaveDriver()
     : m_Initialized(false)
     , m_pHandle(nullptr)
     , m_OwnAddress(0)
     , m_Callback(nullptr)
    {   
    }

    ~I2cSlaveDriver();
    
    /**
     * @brief I2C スレーブドライバの初期化
     * 
     * @param pHandle I2C ハンドル
     * @param ownAddress 自身のアドレス (スレーブアドレス)
     * @param callback Polling で呼び出される受信コールバック
     */
    void Initialize(I2C_HandleTypeDef *pHandle, uint8_t ownAddress, OnReceivedHandler callback) noexcept;

    /**
     * @brief I2C 受信の開始
     */
    void Listen() noexcept;

    /**
     * @brief I2C 受信のポーリング
     * 
     * - メインコンテキストから高頻度で呼び出してください。
     * - I2C スレーブ受信が発生している場合は受信コールバックが呼び出されます。
     * - I2C スレーブ受信が発生していない場合は何もしません。
     * - 受信後送信の場合は受信コールバックが呼び出されるまでクロックストレッチとなります。
     *   (呼び出しが低頻度の場合、バスが長時間ロックされることになります)
     * 
     * @note
     * - 受信後送信の場合はクロックストレッチの効果で受信バッファの上書きは発生しませんが、
     *   受信のみの場合はコールバック中に受信許可状態になるので受信バッファが上書きされる
     *   可能性があります。(TBD)
     */
    void Polling() noexcept;

private:
    bool m_Initialized;
    I2C_HandleTypeDef *m_pHandle;
    uint8_t m_OwnAddress;
    OnReceivedHandler m_Callback;
};

#endif // I2C_SLAVE_DRIVER_H
