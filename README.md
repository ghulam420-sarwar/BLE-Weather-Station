# BLE Weather Station

A battery-powered ESP32 weather station that measures temperature, humidity, and pressure with a BME280, shows readings on a small OLED, and broadcasts them wirelessly over **Bluetooth Low Energy (BLE GATT)**. Connect with the nRF Connect app or the included Python script — no Wi-Fi or internet required.

![Circuit diagram](https://github.com/ghulam420-sarwar/Smart-Home-Automation/blob/main/circuit_diagram.png)

## Highlights

- **BLE GATT server** with three notify characteristics — no pairing needed
- **Button-cycled OLED** pages (temperature / humidity / pressure)
- **Python BLE reader** using `bleak` — logs live data on your laptop
- **Light-sleep friendly** sampling — months of runtime on an 18650

## Hardware

| Component       | Role                      | Interface |
|-----------------|---------------------------|-----------|
| ESP32 DevKit    | MCU + BLE radio           | —         |
| BME280          | Temp / Humidity / Pressure| I²C 0x76  |
| SSD1306 OLED    | 128×32 local display      | I²C 0x3C  |
| Push button     | Cycle display pages       | GPIO0     |
| 18650 + TP4056  | Power + charging          | —         |

## BLE GATT Profile

| Characteristic | UUID (short) | Type    | Unit |
|----------------|--------------|---------|------|
| Temperature    | ...0001      | float32 | °C   |
| Humidity       | ...0002      | float32 | %    |
| Pressure       | ...0003      | float32 | hPa  |

All three support **Read** and **Notify** (2-second interval).

## Build

```bash
pio run -t upload
pio device monitor
```

## Python Reader

```bash
pip install bleak
python python_tools/ble_reader.py
```

```
Scanning for Weather Station...
Found: 24:6F:28:AA:BB:CC
Connected!

   Temp (°C)    Humidity (%)  Pressure (hPa)
--------------------------------------------------
       24.31           41.20          1013.25
       24.32           41.18          1013.24
```

## Testing With nRF Connect

1. Open **nRF Connect** on your phone
2. Scan → find **Weather Station**
3. Connect → expand service `12345678-...`
4. Tap any characteristic → **Read** or enable **Notify**
5. Values arrive as 4-byte little-endian floats

## What I Learned

- Designing a BLE GATT service from scratch (service UUID, characteristic properties, descriptors)
- Encoding floats as 4-byte little-endian for BLE payloads
- Re-advertising automatically after a client disconnects
- Power profiling: active vs. light-sleep current draw comparison

## License

MIT © Ghulam Sarwar
