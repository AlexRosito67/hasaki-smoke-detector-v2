
<p align="center">
  <img src="./logo_kigu.png" width="600" alt="Kigu Logo">
</p>

# hasaki-kigu-quant Smoke Detector

A bare-metal fire detection system combining two tools from the Rosito Bench ecosystem:

- **[Hasaki 刃先](https://github.com/AlexRosito67/hasaki)** — trains the neural network and exports a standalone C header
[Hasaki Pro is available here](https://hasaki.lemonsqueezy.com/checkout/buy/1b6ec0ae-10ec-49dc-8c6f-2af146742a33)

- **kigu-quant** — generates a Q1.15 LUT that replaces the `expf()`-based sigmoid
[Kigu-quant is available here](https://hasaki.lemonsqueezy.com/checkout/buy/a200a9ab-03a4-41d7-9b5f-46da5f150d94)

No TensorFlow. No runtime. No `math.h`. No `expf()`.

---

## The change

The original smoke detector model uses a float32 sigmoid in the output layer:

```c
// Original — requires math.h and expf()
static inline float sigmoid(float x) {
    return 1.0f / (1.0f + expf(-x));
}
```

This repo replaces it with a kigu-quant generated lookup table:

```c
// kigu-quant — no expf(), no math.h
#include "lut_sigmoid.h"
// ...
out[i] = lut_sigmoid_lookup(sum);
```

One `#include`. One function call. `math.h` removed from the project entirely.

---

## Benchmark — ATmega328P, 16MHz, no FPU

| Method | 1000 evaluations | Speedup |
|--------|-----------------|---------|
| `expf()` | 227,292 µs | baseline |
| `lut_sigmoid_lookup()` | 116,512 µs | **1.95x faster** |

Max error vs `expf()`: **0.000021**  
Model accuracy: unchanged (**99.93%** on 7,150 held-out samples)

> On targets without FPU (AVR, Cortex-M0), `expf()` is implemented in software.
> The LUT eliminates that cost entirely — O(1), fixed latency, no transcendental math.

---

## Generating the LUT

The `lut_sigmoid.h` included here was generated with kigu-quant:

```bash
kigu-quant --method lut --func sigmoid --size 256 --fmt q15 -o lut_sigmoid.h
```

---

## Project structure

```
hasaki-kigu-quant-smoke-detector/
├── hasaki_kigu_smoke_detector.ino   Arduino sketch
├── smoke-detector-model-float.h     Hasaki model — sigmoid replaced
├── lut_sigmoid.h                    kigu-quant generated LUT
└── README.md
```

---

## Model

```
Architecture : 12 → 8 (ReLU) → 4 (ReLU) → 1 (Sigmoid LUT)
Parameters   : 169 weights + biases
Flash        : ~3.8 kB
Accuracy     : 99.93% (7,150 unseen samples)
FN rate      : 1 / 3,599 fire events
```

**Input features** (12 sensors):

| # | Feature | Unit |
|---|---------|------|
| 0 | Temperature | °C |
| 1 | Humidity | % |
| 2 | TVOC | ppb |
| 3 | eCO₂ | ppm |
| 4 | Raw H₂ | — |
| 5 | Raw Ethanol | — |
| 6 | Pressure | hPa |
| 7 | PM1.0 | µg/m³ |
| 8 | PM2.5 | µg/m³ |
| 9 | NC0.5 | #/cm³ |
| 10 | NC1.0 | #/cm³ |
| 11 | NC2.5 | #/cm³ |

---

## Part of the Rosito Bench ecosystem

| Tool | Role |
|------|------|
| [Hasaki 刃先](https://github.com/AlexRosito67/hasaki) | Train model → export C header |
| kigu-quant (comming soon) | Generate fixed-point math headers |

**[Rosito Bench](https://github.com/AlexRosito67) — Small tools. Precise work.**

---

*Alex Rosito — Burbank, CA*
