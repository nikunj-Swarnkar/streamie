
### streamie

---

a hackpad or a streaming device with a gamepad in which you can stream your games or play some retro games natively , hack it , add a keyboard and sky is your limit , it runs android !


---

## Cad 

this is the case i prepared 

![final build](image.png)

## Flashing
for the Raxda please follow raxda offical guide , please flash it before assembly 
[Raxda Site](https://docs.radxa.com/en/cubie/a7z/other-system/android)

The controller runs on an RP2040-Zero and flashes by drag-and-drop. No programmer is needed.

### Option 1: flash a prebuilt file
1. Download `controller_xinput.uf2` from the Releases page.
2. Unplug the board from USB.
3. Hold the **BOOT** button and plug the board into your computer. Release BOOT once it's connected.
4. A drive named **RPI-RP2** appears. Drag `controller_xinput.uf2` onto it.
5. The drive disappears and the board restarts as a controller.

### Option 2: build it yourself
Requirements: [VS Code](https://code.visualstudio.com/) with the **Raspberry Pi Pico** extension. It installs the Pico SDK (2.x), compiler and CMake for you.

1. Clone this repo and open the `firmware/xinput` folder in VS Code (**File > Open Folder**). It must be the folder that directly contains `CMakeLists.txt`.
2. In the Pico extension, choose **Import Project** and select that folder.
3. Check that `CMakeLists.txt` has `set(PICO_BOARD waveshare_rp2040_zero)`. Use `pico` if you have a regular Raspberry Pi Pico.
4. Click **Compile Project**. The result is `build/controller_xinput.uf2`.
5. Flash it with the steps from Option 1.

If you use the command line instead, run:
```
mkdir build && cd build
cmake -DPICO_SDK_PATH=/path/to/pico-sdk ..
cmake --build .
```

### Verify it works
1. Keep the **sticks centered and the triggers released** when you plug it in, because they are calibrated at power-up.
2. On Windows, press **Win + R**, type `joy.cpl` and press Enter. The board should appear as **Xbox 360 Controller**.
3. Open **Properties** and check every button, both sticks and both triggers.
4. Pull each trigger fully once so it learns its travel.

### Settings in `main.c`
| Setting | What it does |
|---|---|
| `INVERT_LY`, `INVERT_RY` | Set to `true` if a stick's up and down are backwards |
| `STICK_DEADZONE` | Raise it if the sticks drift |
| `R3_PRESSED_BELOW` | Threshold for the right stick click, which is read through the MCP3008 (CH6) |

### Updating the firmware
Repeat the flashing steps. Always hold **BOOT** while plugging in, even if the board is already running this firmware.

### Troubleshooting
- **No RPI-RP2 drive:** try a different USB cable, because many cables carry power only. Make sure you hold BOOT *before* plugging in.
- **Not recognised as an Xbox controller:** check Device Manager for an unknown device, and try a different USB port.
- **Triggers read constant or backwards:** check the trigger pot wiring. The wiper (middle pad) must go to the MCP3008 channel.
- **Stick axes backwards:** flip `INVERT_LY` or `INVERT_RY` and re-flash.
- **No rumble:** it works only in games and testers that send XInput rumble. Check that the motor MOSFET gates are wired to GP26 and GP27.
- **Compile fails in VS Code:** make sure the open folder is the one containing `CMakeLists.txt`, then run **Clean CMake** and **Compile Project** again.

Compatibility: Windows and Linux. Consoles will not accept this controller.


---

## Assembly 

after printing the case add screen on in it 
![alt text](image-1.png)


then add the front latch to the screen and secure it with 4 M3 screws 
![alt text](image-2.png)

after that add all the buttons from the back 
![](https://cdn.hackclub.com/01a126e3-4b7b-73f5-b9a2-92b3a3756168/journal-1791653792293.png)

then put the pcb on it 
![](https://cdn.hackclub.com/01a126e5-e305-7f71-9732-12c8b52e163b/image.png)

and then rest of the parts and add aux post connected to the screen and add a A7z with stand off and wire everything 
![](https://cdn.hackclub.com/01a126e7-6c70-7aa5-a5bb-b6303b640167/image.png)

connect the controller through usb cable after connecting both side with jst connector , then wire the tp4056 with buck convertor and make sure to set it at 5 v 

after that add the battery to the back and stick it to the back indent with m3 tape or doulbe side tape
![](https://cdn.hackclub.com/01a126f0-59d8-7c74-8348-df9f7221a050/image.png)

after that then just wire it to the tp4056 
and the buckconvertor to the 5v n gnd of the Raxda A7z connect the display with hdmi and if u want touch a usb cable 

after all that 
![](https://cdn.hackclub.com/01a126f4-765b-7aec-b172-44d5268d96f1/image.png)
just screw in the last 4 M3 screws 

![](https://cdn.hackclub.com/01a126f6-317a-752c-b833-f333264660ba/image.png)

and we are done ! 

---


