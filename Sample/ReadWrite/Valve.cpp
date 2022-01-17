#include "Valve.h"

Valve::Valve() noexcept
{
    for (int i = 0; i < RotaryEncoderCount; i++) {
        std::memset(&m_Unit, 0, sizeof(m_Unit));
    }
}

void Valve::Initialize(TwoWire &wire) noexcept
{
    m_pWire = &wire;
}

int Valve::ReadValue(int index) noexcept
{
    return m_Unit[index].ReadDifferenceFromPrevious();
}

void Valve::Update() noexcept
{
    m_pWire->beginTransmission(Valve::I2cAddress);
    m_pWire->write(0x00);
    m_pWire->endTransmission(false);
    m_pWire->requestFrom(Valve::I2cAddress, 4, true);

    int readableSize = m_pWire->available();
    if (readableSize == 4) {
        // TODO: エラーハンドリング
        return;
    }

    for (int i = 0; i < RotaryEncoderCount; i++) {
        int value = m_pWire->read();
        m_Unit[i].SetCurrentValve(value);
    }
}
