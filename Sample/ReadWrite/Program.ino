/*
 * Seeeduino XIAO を I2C マスタとした Valve の読み書きサンプル
 */
#include <Arduino.h>
#include <Wire.h>
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

#define LOG(...) Serial.printf(__VA_ARGS__)

    LOG("setup done!\n");

    g_Wire.begin();
    g_Valve.Initialize(g_Wire);
}

void loop()
{
    // エコーバック
    while (Serial.available()) {
        int r = Serial.read();
        Serial.write(r);
    }
}
