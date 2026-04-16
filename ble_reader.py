"""
BLE Weather Station Reader
--------------------------
Connects to the ESP32 weather station via BLE and prints live readings.

Usage:
    pip install bleak
    python ble_reader.py

Author: Ghulam Sarwar
"""

import asyncio
import struct
from bleak import BleakScanner, BleakClient

DEVICE_NAME  = "Weather Station"
CHAR_TEMP    = "12345678-1234-1234-1234-123456780001"
CHAR_HUM     = "12345678-1234-1234-1234-123456780002"
CHAR_PRES    = "12345678-1234-1234-1234-123456780003"


def decode(data: bytearray) -> float:
    return struct.unpack("<f", bytes(data))[0]


async def main():
    print("Scanning for Weather Station...")
    device = await BleakScanner.find_device_by_name(DEVICE_NAME, timeout=10)
    if not device:
        print("Device not found. Make sure it is powered on and advertising.")
        return

    print(f"Found: {device.address}")
    async with BleakClient(device) as client:
        print("Connected!\n")
        print(f"{'Temp (°C)':>12}  {'Humidity (%)':>14}  {'Pressure (hPa)':>16}")
        print("-" * 50)

        def notify_handler(char_uuid):
            def _handler(_sender, data):
                pass  # used for polling below
            return _handler

        while True:
            t = decode(await client.read_gatt_char(CHAR_TEMP))
            h = decode(await client.read_gatt_char(CHAR_HUM))
            p = decode(await client.read_gatt_char(CHAR_PRES))
            print(f"{t:>12.2f}  {h:>14.2f}  {p:>16.2f}")
            await asyncio.sleep(2)


if __name__ == "__main__":
    asyncio.run(main())
