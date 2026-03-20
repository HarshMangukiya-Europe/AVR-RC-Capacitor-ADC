# 📈 AVR RC Capacitor Charging

> Analysis of RC circuit (5kΩ, 220µF) charging behavior using AVR 
> register-level programming on Arduino Nano.

---

## 📋 Overview
Analog voltage across a capacitor is continuously sampled using **ADC 
in free-running mode**. Five buttons configure different charging and 
frequency modes using direct Timer/Counter register control.
No Arduino libraries used.

---

## ⚙️ Features
| Button | Action |
|--------|--------|
| **S1** | Applies stable 5V charging voltage |
| **S2** | Square-wave at RC cutoff frequency (fc) |
| **S3** | Square-wave at 0.1×fc |
| **S4** | Square-wave at 10×fc |
| **S5** | Manual capacitor discharge |

---

## 🔧 Hardware
- Arduino Nano (ATmega328P)
- RC Circuit: 5kΩ resistor, 220µF capacitor
- 5 push buttons (S1–S5)
- Breadboard + components

---

## 🛠️ Tools Used
- Language: **AVR-C** (no Arduino libraries)
- IDE: **AVR Studio / Microchip Studio**
- Visualization: **Serial Monitor (115200 baud)**

---

## 🎥 Demo Video
[![Watch on YouTube](https://youtu.be/pMYFhm3lSEM?si=e7MwhSE4i86FZRCl)
