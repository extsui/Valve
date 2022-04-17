#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

#include "stm32l0xx_hal.h"
#include "stm32l0xx_hal_gpio.h"

// ロータリーエンコーダで使用する用のポートピン
struct PortPin
{
    GPIO_TypeDef *Port;
    uint16_t Pin;

    explicit PortPin(GPIO_TypeDef *port, uint16_t pin)
    {
        Port = port;
        Pin = pin;
    }
};

class RotaryEncoder
{
public:
    RotaryEncoder()
     : m_phaseA(nullptr, 0)
     , m_phaseB(nullptr, 0)
     , m_IsReversed(false)
     , m_position(0)
     , m_previousValue(0xFF)
     , m_errorCount(0)
    {
    }

    ~RotaryEncoder()
    {
    }

    void SetPortPin(PortPin *phaseA, PortPin *phaseB);

    void SetReverse(bool value = true);

    // A/B 相のサンプリング
    // ポーリング方式なのでノイズ除去が適切に働く周期で呼び出すこと
    void Sample();

    int GetPosition();
    int GetErrorCount();

private:
    // A 相
    PortPin m_phaseA;
    // B 相
    PortPin m_phaseB;

    // 回転方向の反転フラグ
    // (エンコーダの取付方向によっては逆になるため)
    bool m_IsReversed;

    // 起動時からの積算位置
    int8_t m_position;

    // 前回の値 (内部計算用)
    uint8_t m_previousValue;

    // 起動時からの積算エラー回数 (デバッグ用)
    uint32_t m_errorCount;
};

#endif // ROTARY_ENCODER_H
