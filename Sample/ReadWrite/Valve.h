#pragma once

#include <Wire.h>
#include <cstring>

// ロータリーエンコーダ値保持クラス
class RotaryEncoder
{
public:
    RotaryEncoder() noexcept
     : m_PreviousValue(0)
     , m_CurrentValue(0)
    {
    }

    ~RotaryEncoder() = default;

    // 新しい値の設定
    void SetCurrentValve(int value) noexcept
    {
        m_PreviousValue = m_CurrentValue;
        m_CurrentValue = value;
    }

    // 前回からの差分を読み込む
    // 一度読み込むと差分値は無くなる
    // +：正回転
    // -：負回転
    // 0：静止
    int ReadDifferenceFromPrevious() noexcept
    {
        int diff = m_CurrentValue - m_PreviousValue;
        m_CurrentValue = m_PreviousValue;
        return diff;
    }

private:
    int m_PreviousValue;
    int m_CurrentValue;
};

class Valve
{
public:
    // Valve の I2C アドレス
    static constexpr uint8_t I2cAddress = 0x40; // TORIAEZU:
    // ロータリーエンコーダの個数
    static constexpr int RotaryEncoderCount = 4;

private:
    // ロータリーエンコーダの値が連続で格納されている開始レジスタアドレス
    static constexpr uint8_t RegisterAddressRotaryEncoderBegin = 0x00;
    // WHO_AM_I 値が格納されているレジスタアドレス
    static constexpr uint8_t RegisterAddressWhoAmI = 0x80;
    // WHO_AM_I 値
    static constexpr uint8_t RegisterValueWhoAmI = 0xAA;

public:
    Valve() noexcept;
    ~Valve() = default;

    // 初期化
    // Valve の疎通確認も行う
    int Initialize(TwoWire &wire, uint8_t i2cAddress) noexcept;

    // ロータリエンコーダの前回からの差分値を取得
    // TORIAEZU: 前回からの差分値をそのまま返す
    // TODO: 24 クリックタイプだと 1 クリックで ±4 なので加工するとか
    int ReadValue(int index) noexcept;

    // 現在のロータリエンコーダ値の更新
    int Update() noexcept;

private:
    RotaryEncoder m_Unit[RotaryEncoderCount];
    TwoWire *m_pWire;
    bool m_IsInitialized;
    uint8_t m_I2cAddress;
};

