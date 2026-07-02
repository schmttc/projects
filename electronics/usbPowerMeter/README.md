## USB Power Meter

A USB power meter for electronics projects designed to get uA level power readings.

### BOM
- Adafruit INA228
- Arduino Nano
- Nokia 5110 Module (3-5V tolerant)
- USB C breakout board
- USB A breakout board

### Operation
- Updates display every 2s
- Displays readings in approprate scale for
  - Instantaneous Voltage (V)
  - Instantaneous Current (I)
  - Instantaneous Power (W)
  - Running average power (W)
  - Running total energy (Wh)

### Pins

| Common Rail | Arduino Nano Pin | Nokia 5110 Pin | INA228 Pin | USB C (Input) | USB A (Output) |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **GND** | GND | GND | GND | G | GND |
| **3.3V** | - | - | — | - | - |
| **5V** | 5V | VCC | VCC, Vin+ | V | - |
| — | **D2** | CLK | — | - | - |
| — | **D6** | DC | — | - | - |
| — | **D7** | CE | — | - | - |
| — | **D8** | RST | — | - | - |
| — | **D11** | DIN | — | - | - |
| — | **A4** | — | SDA | - | - |
| — | **A5** | — | SCL | - | - |
 |- | - | - | Vin- | - |  VBUS |
  |- | - | - | VBus, Vin+ (bridge) | - | - |