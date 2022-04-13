#if 0

#ifndef I2C_SLAVE_DRIVER_H
#define I2C_SLAVE_DRIVER_H

#include <stdint.h>
#include <string.h>
#include <stm32l0xx.h>
#include "stm32l0xx_hal_i2c.h"

/**
 * 受信専用の I2C スレーブドライバ
 */
class I2cSlaveDriver
{
public:
	I2cSlaveDriver(uint8_t ownAddress);
	~I2cSlaveDriver();
	
	void SetPort(I2C_HandleTypeDef *pI2cHandle) noexcept
	{
		m_pI2cHandle = pI2cHandle;
	}

	int GetReceiveCount() noexcept
	{
		return m_ReceiveCount;
	}

	void EventHandler() noexcept;
	void ErrorHandler() noexcept;

private:
	I2C_HandleTypeDef *m_pI2cHandle;
	uint8_t m_OwnAddress;

	// 受信回数 (デバッグ用)
	int m_ReceiveCount;
};

#endif // I2C_SLAVE_DRIVER_H

#endif
