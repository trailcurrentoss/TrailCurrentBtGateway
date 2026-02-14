# TrailCurrent Bluetooth Gateway

Bluetooth Low Energy (BLE) gateway module that receives commands via Bluetooth and translates them into CAN bus messages for vehicle and trailer control.

## Hardware Overview

- **Microcontroller:** ESP32
- **Function:** BLE-to-CAN bus gateway for remote control of trailer devices
- **Key Features:**
  - BLE with secure bonding (MITM protection)
  - 8 independent device control channels
  - CAN bus interface at 500 kbps
  - Real-time sensor data relay (temperature, solar, battery)
  - Hierarchical PCB schematic design with FreeCAD enclosure

## Hardware Requirements

### Components

- **Microcontroller:** ESP32 with BLE support
- **CAN Transceiver:** Vehicle CAN bus interface (TX: GPIO 15, RX: GPIO 13)
- **LED Indicators:** 8 status LEDs (GPIOs 32, 33, 26, 14, 23, 19, 17, 4)

### KiCAD Library Dependencies

This project uses the consolidated [TrailCurrentKiCADLibraries](https://codeberg.org/trailcurrentopensource/TrailCurrentKiCADLibraries).

**Setup:**

```bash
# Clone the library
git clone https://codeberg.org/trailcurrentopensource/TrailCurrentKiCADLibraries.git

# Set environment variables (add to ~/.bashrc or ~/.zshrc)
export TRAILCURRENT_SYMBOL_DIR="/path/to/TrailCurrentKiCADLibraries/symbols"
export TRAILCURRENT_FOOTPRINT_DIR="/path/to/TrailCurrentKiCADLibraries/footprints"
export TRAILCURRENT_3DMODEL_DIR="/path/to/TrailCurrentKiCADLibraries/3d_models"
```

See [KICAD_ENVIRONMENT_SETUP.md](https://codeberg.org/trailcurrentopensource/TrailCurrentKiCADLibraries/blob/main/KICAD_ENVIRONMENT_SETUP.md) in the library repository for detailed setup instructions.

## Opening the Project

1. **Set up environment variables** (see Library Dependencies above)
2. **Open KiCAD:**
   ```bash
   kicad EDA/trailer-bt-gateway.kicad_pro
   ```
3. **Verify libraries load** - All symbol and footprint libraries should resolve without errors
4. **View 3D models** - Open PCB and press `Alt+3` to view the 3D visualization

### Schematic Sheets

The design uses a hierarchical schematic with dedicated sheets:
- **Root** - Top-level connections
- **Power** - Power distribution and regulation
- **CAN** - CAN bus transceiver interface
- **MCU** - ESP32 microcontroller and support circuits
- **Connectivity** - BLE and connector interfaces

## Firmware

See `src/` directory for PlatformIO-based firmware.

**Setup:**
```bash
# Install PlatformIO (if not already installed)
pip install platformio

# Build firmware
pio run

# Upload to board
pio run -t upload
```

### BLE Service

The gateway exposes a BLE service for device control and sensor data:

- **Service UUID:** `e5553316-4ef1-4a9c-9941-a15f1d2394ea`
- **Device Control:** 8 independent channels with on/off/all control
- **Sensor Data Characteristics:**
  - MPPT solar controller data
  - Temperature and humidity readings
  - Battery shunt monitor data

### CAN Bus Protocol

**Transmit (Gateway to Bus):**
- ID `0x18`: Device control commands (8 bytes)

**Receive (Bus to Gateway):**

| CAN ID | Description |
|--------|-------------|
| 0x1B | Light/device status (8 bytes) |
| 0x1F | Temperature/humidity data |
| 0x23 | Battery shunt data (voltage, current, SOC) |
| 0x24 | Battery wattage data |
| 0x2C | MPPT solar panel data |
| 0x2D | MPPT solar current data |

## Manufacturing

- **PCB Files:** Ready for fabrication via standard PCB services (JLCPCB, OSH Park, etc.)
- **BOM Generation:** Export BOM from KiCAD schematic (Tools > Generate BOM)
- **Enclosure:** FreeCAD design included in `CAD/` directory
- **JLCPCB Assembly:** See [BOM_ASSEMBLY_WORKFLOW.md](https://codeberg.org/trailcurrentopensource/TrailCurrentKiCADLibraries/blob/main/BOM_ASSEMBLY_WORKFLOW.md) for detailed assembly workflow

## Project Structure

```
├── CAD/                          # FreeCAD enclosure design
├── EDA/                          # KiCAD hardware design files
│   ├── trailer-bt-gateway.kicad_pro
│   ├── trailer-bt-gateway.kicad_sch  # Root schematic
│   ├── can.kicad_sch             # CAN subsystem
│   ├── connectivity.kicad_sch    # Connectivity subsystem
│   ├── mcu.kicad_sch             # MCU subsystem
│   ├── power.kicad_sch           # Power subsystem
│   └── trailer-bt-gateway.kicad_pcb  # PCB layout
├── src/                          # Firmware source
│   ├── main.cpp                  # Main BLE/CAN application
│   ├── globals.h                 # Pin definitions and debug macros
│   └── canHelper.h               # CAN bus configuration
└── platformio.ini                # Build configuration
```

## License

MIT License - See LICENSE file for details.

This is open source hardware. You are free to use, modify, and distribute these designs under the terms of the MIT license.

## Contributing

Improvements and contributions are welcome! Please submit issues or pull requests.

## Support

For questions about:
- **KiCAD setup:** See [KICAD_ENVIRONMENT_SETUP.md](https://codeberg.org/trailcurrentopensource/TrailCurrentKiCADLibraries/blob/main/KICAD_ENVIRONMENT_SETUP.md)
- **Assembly workflow:** See [BOM_ASSEMBLY_WORKFLOW.md](https://codeberg.org/trailcurrentopensource/TrailCurrentKiCADLibraries/blob/main/BOM_ASSEMBLY_WORKFLOW.md)
