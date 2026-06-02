# EVerest PEV simulator
The EVerest PEV Simulator project documentation is split in two repositories:
1. this repo: EVerest software running on linux
2. electronics and firmware of the "Dieter" handling low level communication - *" Device Interface ElecTronic Especially not limited to Raspberry"*, available here: [gregsell/XXX]()

*add reference to whole BA?*

## Overview
This project simulates a plug-in EV communicating over CCS with a charging station. It replaces the vehicle with a hardware/software stack that speaks the same protocols:
- **IEC 61851** – Control Pilot (CP) state machine (States A–F, PWM duty cycle)
- **ISO 15118-2** – High-Level Communication (HLC) over Power Line Communication (PLC)
- **DIN SPEC 70121** – Older DC HLC variant (supported by EVerest/Josev)

The software stack is built on [EVerest](https://everest.github.io/), the open-source EV charging framework initiated by PIONIX. EVerest handles the protocol state machines; The module `DieterEvDriver` bridges EVerest to the Dieter hardware board via a serial interface. 

## Architecture
![sw arch](/docs/sys_arch_sw.jpg)
The module `EvManager` is the central connector. `EvSlac` handles the PLC via a homeplug modem and `pyEvJosev` simulates a car. `DieterEvDriver` implements the `ev_board_support` interface, which is required by `EvManager`.

## How to use
### Prerequisites
- EVerest installed according to [documentation](https://everest.github.io/nightly/how-to-guides/getting-started/get-started-sw.html)
- A "Dieter" (compatible) hardware board connected via USB
- A HomePlug PLC modem (patched as EV) (more info [here](https://github.com/uhi22/pyPLC/blob/master/doc/hardware.md))

**environment variables:**
- `$EVEREST_WORKSPACE` : points to `everest-cmake`
- `$EVEREST_PROJECT_DIR`: location of DieterEvDriver

### Build
``` bash
git clone https://github.com/gregsell/DieterEvDriver.git
cd DieterEvDriver/build/

CMAKE_PREFIX_PATH=$EVEREST_WORKSPACE cmake --install-prefix $EVEREST_PROJECT_DIR/dist ..
make DieterEvDriver -j$(nproc) && make install
```

### Configuration
The run configuration `config/run_config.yaml` wires up the modules. Key parameters to adapt to your setup:
```yaml
# Serial port of the Dieter Arduino board
DieterEvDriver:
  config_module:
    serial_port: /dev/ttyUSB0

# PLC network interface for SLAC
EvSlac:
  config_module:
    device: eth1

# the same for pyEvJosev
PyEvJosev:
    config_module:
        device: eth1
```
both can be checked with the following respective commands:
```bash
ls /dev/tty*
nmcli d
```
#### Restrict NetworkManager
As soon as the modem is plugged in, `NetworkManager` tries to assign an IP address via DHCP. This causes problems, because `EvSlac` handles the communication setup by itself.  
The solution is to configure the `NetworkManager` to ignore the respective interface. This can be done by
```bash
# /etc/NetworkManager/conf.d/homeplug-unmanaged.conf
[keyfile]
unmanaged-devices=mac:<mac>
```
and restarting the interface with 
`ip link set up <iface>`

The mac can be retrieved using `ip link show`.

### Run
For each run configuration EVerest generates a run script, if defined in `config/CMakeLists.txt`. The run scripts are located in `build/run-<name>.sh` and start the central `manager` process.

For inspecting traffic [MQTT Explorer](https://mqtt-explorer.com/) and Wireshark using the [dsV2Gshark](https://github.com/dspace-group/dsV2Gshark/) plugin.