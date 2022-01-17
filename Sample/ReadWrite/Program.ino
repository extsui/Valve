#include <Arduino.h>

void setup()
{
    // XIAO の Serial は USB-CDC 用
    Serial.begin(115200);
    // USB ポートが開くまで待機
    // USB-CDC 非搭載のものは常に true
    while(!Serial);

#define LOG(...) Serial.printf(__VA_ARGS__)

    LOG("setup done!\n");
}

void loop()
{
    // エコーバック
    while (Serial.available()) {
        int r = Serial.read();
        Serial.write(r);
    }
}
