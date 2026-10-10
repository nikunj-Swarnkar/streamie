# XInput build (RP2040-Zero + MCP3008)

Appears to Windows as a wired Xbox 360 controller, with working rumble.

## Build
1. Install the Pico SDK (2.x) and `arm-none-eabi-gcc`, `cmake`.
2. Copy `<pico-sdk>/external/pico_sdk_import.cmake` into this folder.
3. ```
   mkdir build && cd build
   cmake -DPICO_SDK_PATH=/path/to/pico-sdk ..
   make -j4
   ```
4. Hold the BOOT button on the RP2040-Zero, plug in USB, drag
   `build/controller_xinput.uf2` onto the `RPI-RP2` drive.

## Tuning (top of main.c)
- `INVERT_LY / INVERT_RY` if a stick axis is backwards. Triggers detect their own direction.
- `STICK_DEADZONE` for stick drift.
- Keep sticks untouched when plugging in (centre is calibrated at boot).
- Keep triggers released at power-up. They learn their full travel as you use them, so pull each fully once after plugging in.

## Notes
- Works natively on Windows and with xpad on Linux. Consoles will not accept it.
- If Windows shows an unknown device instead of "Xbox 360 Controller", tell me
  and send the device-manager details.
