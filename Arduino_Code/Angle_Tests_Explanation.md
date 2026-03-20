# Angle Test Codes Explanation

This document provides a breakdown of the two Arduino test sketches used for measuring and calibrating potentiometer angles in this project.

## 1. AngleTest.ino (Basic Angle Reading & Filtering)

This sketch continuously reads analog signals from three potentiometers, applies a smoothing filter, and converts the raw values directly into 0-360 degree outputs.

### Key Features:
* **Reads 3 Analog Inputs:** Maps to pins `A0`, `A1`, and `A2`.
* **Exponential Moving Average (EMA) Filter:** Applies a mathematical filter (`alpha = 0.15`) to smooth out noisy analog readings from the potentiometers, preventing the final angle values from jittering.
* **Static Mapping & Offset:** 
  * Maps the smoothed ADC values (typically 0 - 4095) directly to angles between 0 and 360 degrees. 
  * Includes a hardcoded `OFFSET_ANGLE` (e.g., 10.0 degrees) that can be adjusted to correct any consistent mechanical misalignment across all sensors.
* **Serial Plotter Ready:** The output is specifically formatted with tabs (`\t`) to be easily graphed in real-time using the Arduino IDE's built-in Serial Plotter.

---

## 2. Angle_Calibration_Test.ino (Dynamic 2-Point Calibration)

This sketch builds upon the filtering concepts of the first script but introduces an interactive, real-time calibration system over the Serial Monitor for two potentiometers. 

### Key Features:
* **Interactive Serial Commands:** While the code is running, you can send specific text commands through the Serial Monitor to tell the ESP32/Arduino what the *current physical angle* of your hardware is:
  * `zero`: Captures the current position for both pots and sets them as the 0-degree baseline.
  * `90`: Captures the current position for Pot 1 and sets it as the 90-degree limit.
  * `25`: Captures the current position for Pot 2 and sets it as the 25-degree limit.
* **Dynamic Range Mapping:** Instead of assuming a fixed 0-4095 range, this code dynamically updates the minimum and maximum mapping points based on the calibration commands you send. 
  * Potentiometer 1 dynamically maps its calibrated points to a `0 - 90` degree range.
  * Potentiometer 2 dynamically maps its calibrated points to a `0 - 25` degree range.
* **Divide-by-Zero Protection:** Includes safety checks in the custom `mapFloat()` function to prevent the program from crashing if the minimum and maximum calibration points accidentally become identical.

### Summary
* Use **`AngleTest.ino`** for quickly checking the raw 360-degree output of up to three sensors.
* Use **`Angle_Calibration_Test.ino`** for interactively fine-tuning specific mechanical limits (0-90° and 0-25°) without needing to repeatedly tweak variables and re-upload the code.
