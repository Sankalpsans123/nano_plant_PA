**This poject includes the following requirements/electronincs:**
1. arduino  nano
2. breadboard(optional)
3. 4X7segemnt display
4. a button(any type)
5. an DTH-11 tempreture sensor
6. an soil moisture sensnor

**Wirings:**
| Component                | Arduino Nano Pin | Notes                                |
| ------------------------ | ---------------- | ------------------------------------ |
| **7-Segment Display**    |                  | (Common Cathode, 4-digit)            |
| Segment A                | `A2`             |                                      |
| Segment B                | `D2`             |                                      |
| Segment C                | `D3`             |                                      |
| Segment D                | `D4`             |                                      |
| Segment E                | `D5`             |                                      |
| Segment F                | `D10`            |                                      |
| Segment G                | `D11`            |                                      |
| Decimal Point (DP)       | `D3`             | Used for blinking colon              |
| Digit 1 (leftmost)       | `D12`            |                                      |
| Digit 2                  | `D9`             |                                      |
| Digit 3                  | `D8`             |                                      |
| Digit 4 (rightmost)      | `D6`             |                                      |
| **DHT11 Temp/Humidity**  | `D13`            | VCC to 5V, GND to GND                |
| **Button**               | `A0`             | Other side of button → GND           |
| **Soil Moisture Sensor** | `A1`             | Digital mode (HIGH = dry, LOW = wet) |
|                          | VCC → 5V         | GND → GND                            |





use this code in arduino ide 
choose the bootloader and the port which you are using
last open serial monitor
set time by the format "set HH MM"
to start chat "start_chat"
to stop chat andback tonormal mode "exit_chat"
