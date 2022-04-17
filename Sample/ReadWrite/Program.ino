/*
 * Seeeduino XIAO を I2C マスタとした Valve の読み書きサンプル
 */

#include "Utils.h"
#include "Valve.h"

namespace {

static TwoWire& g_Wire = Wire;
static Valve g_Valve;

}

void I2cReadWrite(uint8_t deviceAddress, const uint8_t *pTxData, uint8_t txSize, uint8_t *pOutRxData, uint8_t rxSize, uint8_t *pOutRealRxSize)
{
    // 送信フェーズ
    g_Wire.beginTransmission(deviceAddress);
    for (int i = 0; i < txSize; i++) {
        g_Wire.write(pTxData[i]);
    }

    // 受信無しの場合は終了
    if (rxSize == 0) {
        g_Wire.endTransmission(true);
        return;
    }
    g_Wire.endTransmission(false);
    
    // 受信フェーズ
    uint8_t realRxSize = g_Wire.requestFrom(deviceAddress, rxSize);
    for (int i = 0; i < realRxSize; i++) {
        pOutRxData[i] = g_Wire.read();
    }
    *pOutRealRxSize = realRxSize;
}

constexpr uint8_t DeviceAddress = 0x40;
uint8_t g_TxData[8];
uint8_t g_RxData[8];
uint8_t g_RxSize = 0;

void Setup()
{
    memset(g_TxData, 0xFF, sizeof(g_TxData));
    memset(g_RxData, 0xFF, sizeof(g_RxData));
    g_RxSize = 0;
}

void PrintResult()
{
    LOG("Result: rxSize = %d\n  [ ", g_RxSize);
    for (int i = 0; i < g_RxSize; i++) {
        LOG("0x%02x ", g_RxData[i]);
    }
    LOG("]\n");
}

// 想定ケース
// - (S), Address+W, MemAddress, (R), Address+R, ReadData, (P)
// - (S), Address+W, MemAddress, (R), Address+R, ReadData, ReadData, (P)
// - (S), Address+W, MemAddress, (R), Address+R, ReadData, ReadData, ReadData, (P)
// - (S), Address+W, MemAddress, (R), Address+R, ReadData, ReadData, ReadData, ReadData, (P)
// - (S), Address+W, MemAddress, WriteData, (P)
// - (S), Address+W, MemAddress, WriteData, WriteData, (P)
// - (S), Address+W, MemAddress, WriteData, WriteData, WriteData, (P)
// - (S), Address+W, MemAddress, WriteData, WriteData, WriteData, WriteData, (P)
// ※(S) = Stop Condition, (R) = Restart Condition, (P) = Stop Condition
void I2cTest()
{
    // TORIAEZU: デバイス側で受信表示をする影響で間隔をあけておく
    constexpr int TransactionIntervalMilliSeconds = 100;

    LOG("== I2cTest (Read Write) ==\n");

    Setup();
    g_TxData[0] = 0x12;
    I2cReadWrite(DeviceAddress, g_TxData, 1, g_RxData, 1, &g_RxSize);
    PrintResult();

    delay(TransactionIntervalMilliSeconds);

    Setup();
    g_TxData[0] = 0x12;
    I2cReadWrite(DeviceAddress, g_TxData, 1, g_RxData, 2, &g_RxSize);
    PrintResult();

    delay(TransactionIntervalMilliSeconds);

    Setup();
    g_TxData[0] = 0x12;
    I2cReadWrite(DeviceAddress, g_TxData, 1, g_RxData, 3, &g_RxSize);
    PrintResult();

    delay(TransactionIntervalMilliSeconds);

    Setup();
    g_TxData[0] = 0x12;
    I2cReadWrite(DeviceAddress, g_TxData, 1, g_RxData, 4, &g_RxSize);
    PrintResult();

    delay(TransactionIntervalMilliSeconds);

    LOG("== I2cTest (Write Only) ==\n");

    Setup();
    g_TxData[0] = 0xAA;
    g_TxData[1] = 0xBB;
    I2cReadWrite(DeviceAddress, g_TxData, 1 + 1, nullptr, 0, nullptr);

    delay(TransactionIntervalMilliSeconds);

    Setup();
    g_TxData[0] = 0xAA;
    g_TxData[1] = 0xBB;
    g_TxData[2] = 0xCC;
    I2cReadWrite(DeviceAddress, g_TxData, 1 + 2, nullptr, 0, nullptr);

    delay(TransactionIntervalMilliSeconds);

    Setup();
    g_TxData[0] = 0xAA;
    g_TxData[1] = 0xBB;
    g_TxData[2] = 0xCC;
    g_TxData[3] = 0xDD;
    I2cReadWrite(DeviceAddress, g_TxData, 1 + 3, nullptr, 0, nullptr);

    delay(TransactionIntervalMilliSeconds);

    Setup();
    g_TxData[0] = 0xAA;
    g_TxData[1] = 0xBB;
    g_TxData[2] = 0xCC;
    g_TxData[3] = 0xDD;
    g_TxData[4] = 0xEE;
    I2cReadWrite(DeviceAddress, g_TxData, 1 + 4, nullptr, 0, nullptr);

    delay(TransactionIntervalMilliSeconds);

    LOG("Done.\n");
}

void TestValve()
{
    constexpr int TransactionIntervalMilliSeconds = 1000;

    LOG("== Test Valve ==\n");

    LOG("# WhoAmI\n");

    Setup();
    g_TxData[0] = 0x80;
    I2cReadWrite(DeviceAddress, g_TxData, 1, g_RxData, 1, &g_RxSize);
    PrintResult();

    delay(TransactionIntervalMilliSeconds);

    LOG("# GetEncoderValue 1\n");

    Setup();
    g_TxData[0] = 0x00;
    I2cReadWrite(DeviceAddress, g_TxData, 1, g_RxData, 1, &g_RxSize);
    PrintResult();

    delay(TransactionIntervalMilliSeconds);

    LOG("# GetEncoderValue 4\n");

    Setup();
    g_TxData[0] = 0x00;
    I2cReadWrite(DeviceAddress, g_TxData, 1, g_RxData, 4, &g_RxSize);
    PrintResult();

    delay(TransactionIntervalMilliSeconds);

    LOG("# SetReverse 1\n");

    Setup();
    g_TxData[0] = 0x10;
    g_TxData[1] = 0x01;
    I2cReadWrite(DeviceAddress, g_TxData, 2, nullptr, 0, nullptr);
    PrintResult();

    delay(TransactionIntervalMilliSeconds);

    LOG("# SetReverse 4\n");

    Setup();
    g_TxData[0] = 0x10;
    g_TxData[1] = 0x01;
    g_TxData[2] = 0x01;
    g_TxData[3] = 0x01;
    g_TxData[4] = 0x01;
    I2cReadWrite(DeviceAddress, g_TxData, 5, nullptr, 0, nullptr);
    PrintResult();

    delay(TransactionIntervalMilliSeconds);

    LOG("Done.\n");
}

void setup()
{
    // XIAO の Serial は USB-CDC 用
    Serial.begin(115200);
    // USB ポートが開くまで待機
    // USB-CDC 非搭載のものは常に true
    while(!Serial);
    LOG("setup done!\n");

    g_Wire.begin();

    //I2cTest();
    TestValve();

    return;

    int result = g_Valve.Initialize(g_Wire, Valve::I2cAddress);
    // TORIAEZU: 実際の機器がまだないのでエラーになるはず
    LOG("result = %d (expected: -1)\n", result);
}

void loop()
{
    // エコーバック
    while (Serial.available()) {
        int r = Serial.read();
        Serial.write(r);
    }
}
