#include "Utils.h"
#include "Valve.h"

Valve::Valve() noexcept
 : m_IsInitialized(false)
 , m_I2cAddress(0)
{
    for (int i = 0; i < RotaryEncoderCount; i++) {
        std::memset(&m_Unit, 0, sizeof(m_Unit));
    }
}

// @retval  0: 初期化成功
// @retval -1: 通信エラー
// @retval -2: 通信はできたが所望のデバイスではない
int Valve::Initialize(TwoWire &wire, uint8_t i2cAddress) noexcept
{
    ASSERT(!m_IsInitialized);
    int result = 0;

    // WHO_AM_I レジスタを読めるか確認する
    wire.beginTransmission(i2cAddress);
    wire.write(RegisterAddressWhoAmI);
    result = wire.endTransmission(false);
    if (result != 0) {
        return -1;
    }
    // requestFrom() の戻り値は受信バイト数
    result = wire.requestFrom(i2cAddress, 1);
    if (result != 1) {
        return -1;
    }
    uint8_t whoAmI = wire.read();
    if (whoAmI != RegisterValueWhoAmI) {
        return -2;
    }

    m_pWire = &wire;
    m_I2cAddress = i2cAddress;
    m_IsInitialized = true;

    return result;
}

int Valve::ReadValue(int index) noexcept
{
    ASSERT(m_IsInitialized);
    return m_Unit[index].ReadDifferenceFromPrevious();
}

// @retval  0: 更新成功
// @retval -1: 通信エラー
int Valve::Update() noexcept
{
    ASSERT(m_IsInitialized);
    int result = 0;

    m_pWire->beginTransmission(m_I2cAddress);
    m_pWire->write(RegisterAddressRotaryEncoderBegin);
    result = m_pWire->endTransmission(false);
    if (result != 0) {
        return -1;
    }
    result = m_pWire->requestFrom(m_I2cAddress, RotaryEncoderCount, true);
    if (result != RotaryEncoderCount) {
        return -1;
    }
    
    for (int i = 0; i < RotaryEncoderCount; i++) {
        int value = m_pWire->read();
        m_Unit[i].SetCurrentValve(value);
    }
    
    return 0;
}
