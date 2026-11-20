# ESP32 Wi-Fi Controlled 4-DOF Robotic Arm

A four-joint robotic arm controlled from a mobile phone or laptop through a web dashboard hosted by the ESP32. The ESP32 creates its own Wi-Fi access point, so an internet connection is not required. An IR obstacle sensor reports object detection status on the dashboard.

## Features

- ESP32-hosted responsive web dashboard
- Standalone Wi-Fi access point
- Manual control of four servo joints
- Home-position command
- Live IR obstacle detection status
- Live controller connection status
- Mobile and laptop browser support

## Hardware

- ESP32 development board
- Four hobby servo motors compatible with the arm mechanism
- IR obstacle detection module
- External regulated servo power supply
- Breadboard and jumper wires

The uploaded project photograph shows a four-degree-of-freedom arm and an IR obstacle sensor module. The exact ESP32 variant, servo models, power-supply rating and existing wiring cannot be confirmed from the photograph. The pin assignments below are proposed defaults and must be checked against the actual build.

## Pin configuration

| Device | ESP32 GPIO |
|---|---:|
| Base servo signal | 13 |
| Shoulder servo signal | 14 |
| Elbow servo signal | 25 |
| Gripper servo signal | 26 |
| IR sensor digital output | 27 |

The IR sensor is configured as active-low. Change `IR_ACTIVE_LOW` in `src/RoboticArm.ino` if your module reports detection as HIGH.

## Power and wiring

- Connect each servo signal wire to its assigned GPIO.
- Power the servos from a separate regulated supply rated for the combined servo current.
- Connect the external servo supply ground to ESP32 GND.
- Connect the IR module VCC and GND to the appropriate supply rails and its OUT pin to GPIO 27.
- Do not power multiple servos from the ESP32 3.3 V pin or the board's USB supply.
- Verify the servo voltage and current requirements before connecting power.
- Check that the IR sensor output voltage is safe for the ESP32 GPIO.
- Disconnect power before changing wiring.

## Software requirements

- Arduino IDE
- ESP32 board support package
- `ESP32Servo` library

The project uses the ESP32 Arduino core libraries `WiFi` and `WebServer`, which are included with the ESP32 board package.

## Upload

1. Open `src/RoboticArm.ino` in Arduino IDE.
2. Install the `ESP32Servo` library.
3. Select the correct ESP32 board and serial port.
4. Verify the GPIO assignments and power wiring.
5. Upload the sketch.
6. Open Serial Monitor at 115200 baud.

## Connect to the dashboard

1. Connect a phone or laptop to the Wi-Fi network `ArmBot-ESP32`.
2. Enter the password `ArmControl123`.
3. Open `http://192.168.4.1` in a browser.
4. Move the four joint sliders to control the arm.
5. Use **Home position** to command all four joints to 90 degrees.

The access-point name and password can be changed near the top of `src/RoboticArm.ino`.

## Object detection

The IR obstacle module provides a digital detected/not-detected indication. Its detection distance depends on the module, object surface, lighting and potentiometer adjustment. This is proximity/obstacle detection, not camera-based object recognition and not a calibrated distance measurement.

## Mechanical safety

- Test each servo separately with the arm unloaded where possible.
- Confirm the 0–180 degree range is mechanically safe for every joint before operating the complete arm.
- Reduce the angle limits in the firmware if a linkage can bind or collide.
- Keep hands clear of the linkage and gripper while powered.
- Use a stable external power supply; servo current surges can reset the ESP32 if the supply is inadequate.
