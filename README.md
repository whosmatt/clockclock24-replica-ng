# clockclock24-replica-ng
This is the [Vallasc](https://github.com/Vallasc) personal implementation of the "ClockClock 24" by [Humans Since 1982](https://www.humanssince1982.com/), modified my [whosmatt](https://github.com/whosmatt).  
Refer to the git history for a detailed list of changes.  

This fork is a general overhaul with some new features:
- [x] MQTT
- [x] Home Assistant integration and auto-discovery
- [x] Networking overhaul (Configurable hostname w/ mDNS, captive portal on AP mode, robust management)
- [x] Speed customization
- [x] OTA updates (Direct upload or from GitHub CI build)
- [x] CI/CD for OTA updates
- [x] Port to ESP32-C3 and ESP32-S3 (LOLIN ESP32-C3 MINI and LOLIN ESP32-S3 MINI can be used as pin-compatible replacements)
- [x] RGB status LED using onboard addressable LED

## LOLIN ESP32-C3 MINI NOT RECOMMENDED
The RF design of the LOLIN ESP32-C3 MINI is quite bad. Having the board plugged in at the intended spacing reflects back enough power to cause WiFi to be completely unusable, never getting a connection. Raising it with extra spacers or wires helps, but WiFi always ends up lossy and unstable.  
The code contains a softfix where WiFi TX power is set to the minimum (8.5dBm/7mW vs the original 20dBm/100mW) to reduce noise and ringing. It makes the connection usable (still very lossy compared to s3), but reduces range a lot.  
The softfix automatically applies to just the `lolin_c3_mini` target, the `lolin_s3_mini` will use normal full TX power and had a flawless connection with super fast OTA in my testing.  
**The LOLIN ESP32-S3 MINI does not have this issue at all, so it is recommended to use that one instead.**

## 

<div align="center">
<img width="900"  src="/images/photo1.jpg">
</div>

| DISCLAIMER: All files in this repository are intended for personal use only. All design rights go to [Humans Since 1982](https://www.humanssince1982.com/).|
| - |

Clockclock is a kinetic sculpture design by Humans Since 1982, consisting of 24 clocks which by animating show the time in digital format. My attempt to create a replica started by designing the PCB and the case that houses them, and then writing the software to drive the clocks.

---

### Table of Contents: 
1. [Hardware](#hardware)
    1. [Motor](#motor)
    2. [Microcontroller](#microcontroller)
    3. [PCB](#pcb)
2. [Software](#software)
    1. [Master](#master)
    2. [Slave](#slave)
    3. [Web Interface](#web-interface)
    4. [Hands calibration](#hands-calibration)
3. [Case and clock's hands](#case-and-clocks-hands)
4. [Final result](#final-result)

---

## Hardware
The entire clock is composed of 24 sub-clocks, arranged on a 8x3 matrix, that are fully independent. A module that contains 3 clocks and can show vertically an half digit has been designed. A total of 8 boards are used to make up the entire clock, nothing prohibits adding more, thus increasing the number of digits.

### Motor
After careful research, *VID28-05* (or *BKA30D-R5*) stepper motor was chosen. It is made by two separate motors that drive the two shafts individually, that is perfect for attaching the hands. Moreover, this type is sold by some vendors (search on Aliexpress) with no endstops and can turn freely.

<img align="right" width="400"  src="/images/vid-28.gif">

These motors are low power and can be runned directly by the microcontroller, but in order not to stress the GPIO a dedicated controller was used. Specifically the *AX1201728SG* (equivalent of *X12.017* and *VID6606* chips) offers advantages over running the motor directly such as uses of microstepping. It requires only two GPIO pins per motor, protects the microprocessor from the inductive effects of the motor coils and requires lower current to be runned by the microcontroller.

### Microcontroller
Two types of microcontrollers are used: **Raspberry-pi-pico** (RP2040) and **ESP32**. The main reason for this choice is that ESP32 has wifi capabilities, the design was made before the release of Raspberry-pi-pico-w, now the PCB could be simplified even more. ESP32 acts as a master and sends commands to the pico boards that run directly the motors. Raspberry-pi-pico was chosen because it has a lot of pins, it is reliable and it is available on the market at a relatively low price.

### PCB
The schematic and all the gerber files are placed in the *./pcb* directory. 
<div align="center">
<img align="center" width="800"  src="/images/pcb.jpg">
</div>

#### Schematic
<div align="center">
<img width="800"  src="/images/schematic.jpg">
</div>
Each board has 2 stepper controllers that in total can run 8 motors. As a design choice, the board has only 3 clocks (6 motors), so 2 outputs are unused. The schematic also contains 6 hall sensors which were to be used to automatically adjust the position of the hands, but in the end they were not added to the final prototype because the magnets were not powerful enough to trigger the sensors. Each board has a ESP32 footprint but it should only be installed on one (any) board (master). This makes it possible to produce only one type of printed circuit board to which components will be soldered according to purpose. Master talks to slaves using I2C protocol, each board is connected to the next in daisy chain. All the boards are powered with 5V and should approximately consume at peak times 1.7A.

#### Bill Of Materials (full clock)
<div align="center">

| Quantity | Name | 
|-|-|
| 8 | PCB board |
| 24 | VID28-05 (or BKA30D-R5) 360 degree mod |
| 16 | AX1201728SG (or VID6606) |
| 8 | Raspberry pi pico |
| 1 | ESP32 MINI (LOLIN ESP32-C3 MINI or LOLIN ESP32-S3 MINI)|
| 8 | DIP switch 4 switches |
| 16 | JST XH 2.54 MALE 4 pins |
| 14 | JST XH 2.54 FEMALE 4 pins (for connection cables) |
| 8 | (Optional) power connector |

</div>

---

## Software
On the software side, two different projects were made for master and slave, [PlatformIO](https://platformio.org/)  was used for both of them. The project setup is easy with PlatformIO because it automatically downloads the necessary files. 

### Slave
The slave code is runs on a Raspberry Pi Pico, it receives the target hands position via I2C and drives the motors via step/dir. The I2C address is taken from the position of the 4 switches on the board.

To have a fluid animation, motion must be done using an acceleration curve, so it is used [AccelStepper](http://www.airspayce.com/mikem/arduino/AccelStepper/) library. 
The code is multicore, one core gets bytes from the I2C bus and saves them in the internal buffer, the other core runs the stepper motors, doing so the animation is not stopped by the I2C interrupts.


### Master
The master code runs on an ESP32, it serves the web application and sends the target hand positions with speed acceleration and movement strategy to the slave boards.

When powered on, it tries to connect to the configured WiFi network, with fallback to AP mode.  
Whenever you visit the web app, time and time zone are synchronized from your browser. Otherwise, NTP is used to continuously keep time.  
The web app is available at http://clockclock24.local (uses your configured hostname) or http://192.168.1.10 (AP mode only). Your router may also assign a DNS entry for the clock using the hostname, such as http://clockclock24/ (or the respective FQDN).  
In AP mode, a captive portal is active, and most devices should automatically send you to the web app.

### Flashing (First installation or recovery)

You can get the latest compiled binary for your board from the releases page and flash it directly via esptool or an ESP32 web flasher. You can also clone the project and compile/flash via PlatformIO.  
Check a basic guide on flashing ESP32 devices to learn more about the boot button and USB drivers. 

#### Status LED
The onboard addressable RGB LED is used as a status indicator:
| Color | Status |
|-|-|
| Green | Connected to WiFi |
| Red | Critical Failure - Check serial logs |
| Blue (blinking) | Connecting to WiFi |
| Cyan | AP mode active |
| Yellow (blinking) | OTA update in progress |
| Yellow (solid) | OTA update successful, rebooting |


### Web interface
On the web application an approximate copy of the clock is shown. The interface allows you to change mode, configure settings, fine-adjust hands and do OTA updates.  

#### ON/OFF and Mode

Animation modes available (for now):
1. **Lazy**, moves only clock hands that need to be changed by traveling the minimum distance.
2. **Fun**, moves all clocks in a clockwise direction.
3. **Waves**, All clocks straighten out and then follow a staggered domino animation. Produces interesting patterns at high speed settings.

#### Speed Multiplier
The speed multiplier is a global setting that multiplies the speed and acceleration of all animations. It is uncapped, but values above 50-100 may cause the motors to miss steps and lose position. You can only make the clock faster and only integer values are accepted.

#### Sleep Time
To reduce wear and noise during absence and night time, the clock can automatically turn off at the hours configured in the sleep time settings.

#### MQTT and Home Assistant
Configuring MQTT allows you to control the mode and on/off state via MQTT. Home Assistant auto-discovery is supported, and the clock will appear as a device with two entities: a select for the mode and a switch for the on/off state. The Sleep Time state will override the on/off state. 

#### Firmware Update
You can update the firmware directly from the web interface. You can upload your own firmware.bin (make sure to use the correct target), or you can use the `UPDATE FROM GITHUB` button to automatically install the latest automated build form this repository if the clock is connected to the internet.

Credits for the clock's web design animation go to [Manuel Wieser](https://manu.ninja/).

<div align="center">
<img width="500"  src="/images/web.jpg">
</div>

### Hands calibration
Before powering on the clock, you need to manually set all the clock's hands at 6 o'clock. This process needs to be done every time it is disconnected from power.

---

## Case and clock's hands
The case is one piece, it was made by taking a sheet of black MDF and hollowing out the material with a CNC machine. This gives a lot of stiffness to the structure. The clock's hands are laser cutted from a white plexiglass panel.

Since I don't want to receive a letter from Humans Since 1982’s lawyers, all design files and the actual clock assembly are omitted, more [here](https://hackaday.com/2020/07/31/patent-law-and-the-legality-of-making-something-similar/).
> “Humans Since 1982 have the copyright to works displaying digital time using a grid arrangement of analog clocks…”

---

## Final result
<div align="center">

| Animation type | Result |
| - | - |
| Lazy | ![sasdasd](/images/clock1.gif) |
| Fun | ![sasdasd](/images/clock2.gif) |
| Waves | ![sasdasd](/images/clock3.gif) |

</div>
