# <a name="Main Lift Controller"></a>Main Lift Controller #

This is the software for the Main Lift Controller.
The software has been tested on the following lift decoder boards:
  - SMD board: https://oshwlab.com/aikopras/support-lift-controller
  - THT board: https://oshwlab.com/aikopras/lift-decoder-arduino-mega-tht

Instructions for compiling can be found on:
  - SMD board: https://github.com/aikopras/Lift_Vitrine/blob/main/extras/Board-SMD/Compile.md
  - THT board: https://github.com/aikopras/Lift_Vitrine/blob/main/extras/Board-THT/Compile.md

## Modes of operation ##
The Main Lift Controller can be manually and/or automatically operated:
- Manually, via pushing a button that is attached to the button controller.
- Automatically, by sending a DCC Accessory (switch) Command.

### LEDs ###
The LEDs have the following meaning:
- Blue: The blue LED blinks whenever the stepper motors are active.
- Green: The green LED indicates that power is connected and the IR sensors are free (or disabled).
- Yellow: The yellow LED mimics the LEDs on the remote button panel (without flashing).
- Red: The red LED blinks twice during startup, and once whenever a RSbus feedback message is send (see the feedback section below). If no DCC address has been connected yet, the red LED blinks continuously.


### Buttons ###
If the lift is not moving, the following button behavior is possible:
 - Short press of one of the level (0..11) buttons: Lift starts moving to the requested level.
 - Short press of the RESET button: a homing cycle will be initiated.
 - Press of ^ or v buttons: Lift moves in small steps up or down (jogging).
 - Long press of one of the level buttons: the current position will be stored as new the position for that level

While the lift moves, the associated button LED will slowly flash.
Once the move is complete, the button LED will shortly light and subsequently dim. If a new position is being stored, the button LED will flash quickly.

If the lift is moving, all buttons are ignored, except the RESET button. If the RESET button is pushed (short press), the lift will immediately be stopped (see emergence stop below).



### DCC address ###
The common approach for all my DCC decoder boards, is to compile the DCC address with an "illegal" value. This triggers the board to notify the user, via blinking of the red LED, that a DCC address needs to be entered. Like all other decoder boards, this DCC address can be set using the onboard programming button. This new address is permanently stored (in EEPROM), and the next time the decoder starts, the valid DCC address is read from EEPROM and the red LED no longer blinks.

However, you might prefer to set the DCC address already at compile time. This can be done by changing the CV1 and CV9 settings in the file [mySettings.h](mySettings.h)


### RS-Bus address ###
The onboard programming button is not only used to set the DCC address, but indirectly also the RS-Bus address. If you select a decoder address < 128 (thus a switch addresses <509), the RS-Bus address becomes equal to the decoder address (+1). If you select a decoder address >=128, the RS-Bus address becomes 0, meaning "not used". Via the onboard programming button it is therefore not possible to use decoder addresses >=128, in combination with RS-Bus feedback.

Fortunately, we can still use DCC decoder addresses >=128, in combination with RS-Bus feedback, provided we select the RSBus addresses, before compilation, in the file [mySettings.h](mySettings.h).


### Feedback ###
Feedback is provided regarding the lift's position and status. There are twelve level and three status bits. During movement of the lift, all feedback bits are cleared. The three status bits indicate if:
- the IR-sensors are free (provided IR-sensors are active)
- the Lift has arrived / is at level x. There is no movement and the stepper motors are idle
- the lift is ready.<BR>
If the IR-Sensors are active, this bit is the same as 'IR-sensors are free' AND 'Lift is at level x'.<BR>
If the IR-Sensors are inactive, this bit is the same as 'Lift is at level x'.
Analyzing this single bit may, in TrainControl (or whatever software you have), be easier than analyzing two bits.


Feedback is provided in two ways.
1. Using the RS-Bus. We use two addresses; the meaning of the individual bits, is as follows:
 - Base address    : Level 0..7                (low and high nibble. Bit 0..7)
 - Base address + 1: Level 8..11               (low nibble.          Bit 0..3)
 - Base address + 1: IR-sensors are free       (high nibble          Bit 4)
 - Base address + 1: Lift is at level x        (high nibble          Bit 5)
 - Base address + 1: Lift Ready                (high nibble          Bit 7)
2. Using the connectors on the Main Lift Board connectors (added in V2.0).<BR>
Its purpose is to facilitate the use of alternative feedback systems (such as S88).
  - The connectors labelled "IN 1..12" are used to tell which level the lift currently is.
  The connector labelled "1" is for level "0", etc.
  - The connector labelled "IN 13" is used to tell that IR-sensors are free.
  - The connector labelled "IN 14" is used to tell that lift has arrived / is  at level x.
  - The pin labelled "OUT 1" is to tell that the lift is ready.

Note that the IN connectors are directly connected to pins on the ATMega 2560 processor, (accidental) shortcut of these pins will destroy the processor.
Therefore ensure you always use resistors with a value of 1 kOhm or higher.

### Emergency stop ###
If the lift is moving, it can be stopped by either pushing the RESET button, or by sending a DCC Emergency Stop command (= Broadcast ResetCmd). In both cases the RESET button will quickly flash.
1. BUTTON: The Button generates a soft_reset, which immediately halts the steppers and resets GRBL. If reset while in motion, GRBL will throw an Alarm to indicate position may be lost from the motion halt. The Alarm state is signaled via the RESET Button LED, which quickly flashes. A second push of the RESET button is needed to leave the Alarm state. The lift stays at an  undefined position.
2. DCC Emergency Stop command: generates a GRBL feedhold, This allows to lift to continue its movement afterwards. In case the lift should not restart its movement after the DCC emergency stop, while still in the HOLD state, the reset-button which will trigger a soft-reset.


# Initialization #
Before the Main Lift Controller can be used, a number of settings must first be made in the file [mySettings.h](mySettings.h).

##### 1) Tell the sketch which board will be used #####
As mentioned at the top of this page, the lift controller code has been tested on a SMD and a THT board. Although the decoder software will also run on other boards, it is important that the processor is an ATMega 2560 and that the correct connections are made between the processor pins and the various peripherals. Some of these assignments are made in the file [hardware.h](hardware.h) (the DCC, RSBus and RS485 assignments are made by their respective libraries). As can be seen in that file, some small differences exists between the pin assignments for both boards. The reason behind these differences, is that the SMD board  was developed first, and some pins were used that are not available on standard Arduino MEGA boards. Later the THT board was developed, which made some pin changes necessary. The choice between both boards is made via a `#define`.

To use the SMD board, enable the associated `#define`.
```
    #define BOARD_SMD
```
For the THT board, enable the other `#define`:
```
    #define BOARD_THT
```
##### 2) Homing #####
By default, a homing cycle for the stepper motor(s) is performed at program start. Such homing cycle ensures that the lift will move to a defined position. Uncomment the `#define` (remove the starting //) if homing is NOT desired.
```
    #define NO_HOMING
```

##### 3) IR-Sensors #####
By default, every time before the lift starts moving, the IR-sensors connected to the dedicated IR-Sensor Board will be checked. If a train blocks an IR-beam, the lift will not move. Uncomment the `#define` if the IR-sensors should not be checked. This may be required for testing purposes, or if no IR_Sensor board is connected and needed.
```
    #define NO_IR_SENSORS
```

##### 4) LCD Display #####
By default, the LCD is disabled, to avoid interference with RS-bus feedback and other time critical functions. Such interference is caused by the fact that writing to the LCD screen takes quite some time. For testing purposes, or in cases where time critical functions like RS-bus feedback are not needed, the LCD display may be enabled.
```
    #define ENABLE_LCD
```

##### 5) SERIAL_MONITOR #####
By default, the serial monitor interface is switched off (value = 0).
For entering GRBL commands or debugging, it may be convenient to enable the serial monitor.<BR>
If the value = 1, input from the serial line will be copied to the GRBL processor, and information gets displayed regarding the current lift position.<BR>
If the value = 2, input from the serial line will be copied to the GRBL processor, and all data coming back from the GRBL processor gets displayed.
```
    #define SERIAL_MONITOR 1
```

##### 6) DCC Address #####
The decoder can listen to DCC accessory commands to move the lift to a certain level. Each lift level has its own switch address; the first switch address is for level 0; the second switch address is for level 1, the third for level 2, etc.<BR>
Per 4 switch addresses we need one decoder address. Since the maximum number of lift levels is 12, we listen to three decoder addresses.

The first decoder address is stored in CV1 plus CV9. The relationship between CV1, CV9  and the decoder address is explained in RCN-213 (Section 2.1) and RCN-225.
- the valid range for CV1 is 1..63 (if CV9 == 0) or 0..63 (if CV9 !=0)
- the valid range for CV9 is 0..3  (or 128, if the decoder has not been initialized).

If you don't `#define` these CVs here, the default value for CV1 (myAddrL) = 1 and for CV9 (myAddrH) = 128 (0x80). A CV9 value of 128 indicates that the address has not been set by the user yet (by pushing the button).
In that case the decoder's red LED starts blinking.

If your lift will not  be controlled by DCC, but only manually by buttons, you should change CV9 to 0.

Uncomment both `#defines` below and fill-in the address CVs yourself
```
    #define CV1 56
    #define CV9 3
```

##### 7) Set the RS-Bus addresses #####
The RS-Bus address is stored in CV10 (myRSAddr). Valid addresses are between 1..128. The default value is 0, meaning that the RSbus becomes inactive.
The RS-Bus address 128 is used by all my decoders for PoM feedback.
We need two RS-Bus addresses for all feedback information; only the first address needs to be entered below. This address should therefore be between 1..126.
```
    #define RS_ADDRESS 126
```

#### 8) Relays ####
The decoder board allows the connection of two (bi-stable) relays. These relays can, for example, be used to:
1. ensure that the track that connects the lift to the remaining tracks, will only be powered whenever the lift is at level 0. This would be an additional safety measure
2. allow a change of boosters, depending if the lift is at level 0 or at another level. This avoids potential problems at the border between two booster sections

In case the lift is IDLE and at level 0, both relays will be switched to "position 1". In all other cases both relays will be switched to "position 2".

You may change the pins for external relays, provided they stay somewhere on the OUT 9..14 pins (Port K). If you don't use any relays, just keep the values below.
```
    #define RELAY1_POS1    63  // PIN_PK1 - Number on PCB: OUT 10
    #define RELAY1_POS2    64  // PIN_PK2 - Number on PCB: OUT 11
    #define RELAY2_POS1    65  // PIN_PK3 - Number on PCB: OUT 12
    #define RELAY2_POS2    66  // PIN_PK4 - Number on PCB: OUT 13
```

*Note:* The Lift-decoder outputs become, once activated, low. The load should therefore be connected between the output and +5V. Once activated, the differential voltage becomes something like 4 Volt. This might be enough for a 5 (or 3,3V) relay, but certainly not for a 12V relay. Therefore the connection towards 12V relays should be performed via optocouplers, which "translate" between the 5V output domain, and a separate 12V domain for the relays.   


#### 9) Initial lift positions ####
Initial lift positions. Will be entered into EEPROM if and only if the EEPROM has not been initialized. Once the EEPROM is initialized, values will not be written to EEPROM again, even if you make changes in [mySettings.h](mySettings.h). Later changes regarding lift positions should be made via the buttons.

In case you don't have buttons (since the lift is operated via DCC only), you can enable FORCE_EEPROM_WRITE (see below).
After these values are stored in EEPROM, do NOT modify the values again!! Values are in millimeter (point as decimal separator).
```
    #define LEVEL00       "0.000"
    #define LEVEL01     "100.000"
    #define LEVEL02     "200.000"
    #define LEVEL03     "300.000"
    #define LEVEL04     "400.000"
    #define LEVEL05     "500.000"
    #define LEVEL06     "600.000"
    #define LEVEL07     "700.000"
    #define LEVEL08     "800.000"
    #define LEVEL09     "900.000"
    #define LEVEL10    "1000.000"
    #define LEVEL11    "1100.000"
```

#### 10) Force EEPROM write ####
Set the `#define` below to `1`, if the new values MUST be written to EEPROM. Don't forget to change it back to `0` once the new settings are stored in EEPROM, to avoid EEPROM wear-out.
```
    #define FORCE_EEPROM_WRITE 0
```
