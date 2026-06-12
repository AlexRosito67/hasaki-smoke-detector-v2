/**
 * hasaki-kigu-quant Smoke Detector Demo
 * Rosito Bench — Small tools. Precise work.
 *
 * Neural network inference with kigu-quant LUT sigmoid.
 * No expf(). No math.h. No runtime.
 *
 * Model:    12-8-4-1 (relu, relu, sigmoid)
 * Trained:  Hasaki 刃先
 * Sigmoid:  kigu-quant LUT Q1.15 — replaces expf()
 *
 * Board: ESP32-C3 Super Mini (tested) / Arduino Nano / any Arduino-compatible
 */

#include <Arduino.h>
#include "smoke-detector-model-float.h"  // includes lut_sigmoid.h internally

#define ALARM_PIN         3
#define TRIGGER_THRESHOLD 3

// Input normalization ranges (from training data)
const float INPUT_MIN[12] = {
    -22.01f,  10.74f,      0.0f,      400.0f,
    10668.0f, 15317.0f,  930.852f,    0.0f,
      0.0f,     0.0f,      0.0f,      0.0f
};
const float INPUT_MAX[12] = {
     59.93f,  75.2f,   60000.0f,  60000.0f,
    13803.0f, 21410.0f,  939.861f, 14333.69f,
    45432.26f, 61482.03f, 51914.68f, 30026.438f
};

void setup() {
    Serial.begin(115200);
    pinMode(ALARM_PIN, OUTPUT);
    delay(500);
    Serial.println(F("hasaki-kigu-quant Smoke Detector"));
    Serial.println(F("sigmoid: lut_sigmoid_lookup() — no expf()"));
}

void loop() {
    // Replace with actual sensor readings
    float input[12] = {
        read_temp(),     read_hum(),      read_tvoc(),    read_eco2(),
        read_rawh2(),    read_ethanol(),  read_pressure(),
        read_pm1(),      read_pm25(),     read_nc05(),
        read_nc1(),      read_nc25()
    };

    // Normalize inputs to [0.0, 1.0]
    for (int i = 0; i < 12; i++)
        input[i] = (input[i] - INPUT_MIN[i]) / (INPUT_MAX[i] - INPUT_MIN[i]);

    // Inference
    float output[1];
    predict(input, output);

    // State accumulator — requires TRIGGER_THRESHOLD consecutive positives
    static int accumulator = 0;
    if (output[0] >= 0.5f) {
        if (accumulator < TRIGGER_THRESHOLD) accumulator++;
    } else {
        if (accumulator > 0) accumulator--;
    }

    digitalWrite(ALARM_PIN, accumulator >= TRIGGER_THRESHOLD ? HIGH : LOW);

    Serial.print(F("output: ")); Serial.print(output[0], 4);
    Serial.print(F("  alarm: ")); Serial.println(accumulator >= TRIGGER_THRESHOLD ? "YES" : "no");

    delay(1000);
}
