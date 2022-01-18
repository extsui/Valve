/*
 * Seeeduino XIAO を I2C マスタとした Valve の読み書きサンプル
 */

#include "Utils.h"
#include "Valve.h"

namespace {

static TwoWire& g_Wire = Wire;
static Valve g_Valve;

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
