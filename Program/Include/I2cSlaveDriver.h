#ifndef I2C_SLAVE_DRIVER_H
#define I2C_SLAVE_DRIVER_H

#include <stdint.h>
#include <string.h>
#include <stm32l0xx.h>
#include "stm32l0xx_hal_i2c.h"

/**
 * @brief I2C スレーブドライバ
 * 
 * - 送受信対応
 * - 正常系のみで異常系はほぼ考慮外 (TBD)
 * - I2C スレーブが 1 システムに複数あるのは考えにくいので 1 インスタンスのみに制限
 * - 受信コールバックは割り込みコンテキストで呼び出される
 *   - クロックストレッチでマスタ側を長時間待たせないようにするため
 */
namespace I2cSlaveDriver {

typedef int (*OnReceivedHandler)(uint8_t *pOutTxData, int *pOutTxSize, const uint8_t *pRxData, int rxSize);

/**
 * @brief I2C スレーブドライバの初期化
 * 
 * @param pHandle I2C ハンドル
 * @param ownAddress 自身のアドレス (スレーブアドレス)
 * @param onReceivedHandler Polling で呼び出される受信コールバック
 */
void Initialize(I2C_HandleTypeDef *pHandle, uint8_t ownAddress, OnReceivedHandler onReceivedHandler) noexcept;

/**
 * @brief I2C 受信の開始
 */
void Listen() noexcept;

}

#endif // I2C_SLAVE_DRIVER_H
