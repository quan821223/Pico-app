# Pico-app

This is a private repo project for Raspberry Pi Pico application.

## Build prerequisites

Before building, confirm these tools and environment variables are available in the current terminal.

```powershell
cmake --version
arm-none-eabi-gcc --version
$env:PICO_SDK_PATH
Test-Path "$env:PICO_SDK_PATH\external\pico_sdk_import.cmake"
```

Expected result:

- `cmake --version` prints a CMake version.
- `arm-none-eabi-gcc --version` prints the GNU Arm Embedded Toolchain version.
- `$env:PICO_SDK_PATH` points to the Pico SDK directory.
- `Test-Path ...pico_sdk_import.cmake` returns `True`.

For this machine, the known Pico SDK path is:

```powershell
$env:PICO_SDK_PATH="D:\YQRepo\pico\pico-sdk"
```

The SDK path must point to the directory that contains `external\pico_sdk_import.cmake`.

If `arm-none-eabi-gcc` is not found, add the ARM toolchain `bin` folder to the current terminal:

```powershell
$env:Path = "C:\Program Files (x86)\GNU Arm Embedded Toolchain\10 2021.10\bin;" + $env:Path
arm-none-eabi-gcc --version
```

If Git blocks the Pico SDK with a `dubious ownership` error, run:

```powershell
git config --global --add safe.directory D:\YQRepo\pico\pico-sdk
```

## Build with Ninja

Use this method if `ninja` is installed and available in `PATH`.

Check Ninja:

```powershell
ninja --version
```

Configure and build:

```powershell
cd <PROJECT_ROOT>

$env:PICO_SDK_PATH="D:\YQRepo\pico\pico-sdk"
$env:Path = "C:\Program Files (x86)\GNU Arm Embedded Toolchain\10 2021.10\bin;" + $env:Path

Remove-Item build -Recurse -Force -ErrorAction SilentlyContinue

cmake -S . -B build -G Ninja -DPICO_COMPILER=pico_arm_cortex_m0plus_gcc
cmake --build build
```

Important: `cmake --build build` only compiles an already configured build directory. If `build\build.ninja` does not exist, run the `cmake -S . -B build -G Ninja ...` command first.

Equivalent commands when already inside the `build` directory:

```powershell
cmake -G Ninja -DPICO_COMPILER=pico_arm_cortex_m0plus_gcc ..
ninja
```

## Build with NMake

Use this method from a Visual Studio developer terminal, such as:

- `Developer PowerShell for VS 2022`
- `x64 Native Tools Command Prompt for VS 2022`
- Visual Studio Build Tools developer prompt

Check NMake:

```powershell
nmake /?
```

Configure and build:

```powershell
cd <PROJECT_ROOT>

$env:PICO_SDK_PATH="D:\YQRepo\pico\pico-sdk"
$env:Path = "C:\Program Files (x86)\GNU Arm Embedded Toolchain\10 2021.10\bin;" + $env:Path

Remove-Item build -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory build | Out-Null
cd build

cmake -G "NMake Makefiles" -DPICO_COMPILER=pico_arm_cortex_m0plus_gcc ..
nmake
```

Important: if CMake output says `The C compiler identification is MSVC`, the build directory is configured incorrectly for Pico. Delete `build`, make sure `arm-none-eabi-gcc --version` works, then configure again. Pico should use ARM GCC, not MSVC `cl.exe`.

## Common environment problems

### Pico SDK is missing

Error example:

```text
include could not find requested file:
  /external/pico_sdk_import.cmake

Unknown CMake command "pico_sdk_init".
```

Cause: `PICO_SDK_PATH` is empty or wrong.

Fix:

```powershell
$env:PICO_SDK_PATH="D:\YQRepo\pico\pico-sdk"
Test-Path "$env:PICO_SDK_PATH\external\pico_sdk_import.cmake"
```

The test must return `True`.

### ARM compiler is missing

Error example:

```text
Compiler 'arm-none-eabi-gcc' not found
```

Cause: GNU Arm Embedded Toolchain is not installed or its `bin` folder is not in `PATH`.

Fix:

```powershell
$env:Path = "C:\Program Files (x86)\GNU Arm Embedded Toolchain\10 2021.10\bin;" + $env:Path
arm-none-eabi-gcc --version
```

### NMake is missing

Error example:

```text
The term 'nmake' is not recognized
```

Cause: the terminal did not load the Visual Studio build environment.

Fix: open `Developer PowerShell for VS 2022` or `x64 Native Tools Command Prompt for VS 2022`, then run:

```powershell
nmake /?
```

If `nmake` is found, rerun the NMake build flow from a clean `build` directory.

### Ninja is missing

Error example:

```text
The term 'ninja' is not recognized
```

Cause: Ninja is not installed or not in `PATH`.

Fix:

```powershell
winget install Ninja-build.Ninja
```

Close and reopen PowerShell, then check:

```powershell
ninja --version
```

### Build directory has the wrong generator or compiler

If you previously configured `build` with NMake/MSVC and then switch to Ninja/ARM GCC, do not reuse the same cache.

Always reset the build directory when changing generators or compiler settings:

```powershell
cd <PROJECT_ROOT>
Remove-Item build -Recurse -Force -ErrorAction SilentlyContinue
```

Then configure again with either the Ninja or NMake flow above.

## Build output

After a successful build, check the `build` directory for generated files:

```powershell
Get-ChildItem build -Recurse -Filter *.uf2
```

The `.uf2` file is the firmware image to copy to the Raspberry Pi Pico boot drive.

## UART chamber status backdoor

The firmware has a UART backdoor for testing the `DA 52 20 ...` chamber status response without wiring the real chamber address pins.

Hardware UART:

- UART: `uart0`
- TX: `GP16`
- RX: `GP17`
- Baud rate: `115200`

Normal behavior:

- `MSGaddress_TYPE_20` returns the chamber status from `CHAMBER_ADDR_PIN1` and `CHAMBER_ADDR_PIN0`.
- `CHAMBER_ADDR_PIN1` is bit 1.
- `CHAMBER_ADDR_PIN0` is bit 0.

Backdoor behavior:

- When the backdoor is disabled, the firmware reads the real GPIO pin levels.
- When the backdoor is enabled, the firmware ignores the real GPIO pin levels and returns the simulated value set through UART.

Send these commands as raw HEX bytes through UART RX, not as ASCII text:

When the Pico receives a supported UART backdoor command, it replies on UART TX with:

```text
C3 0D 0A
```

Command format:

```text
DA 57 03 XX YY
```

- `DA`: command header
- `57`: write command
- `03`: target device/type field used by this backdoor
- `XX`: backdoor command address
- `YY`: command value

Backdoor enable command:

```text
DA 57 03 FF YY
```

- `FF`: backdoor enable control
- `00`: disable backdoor and return to real GPIO pin detection
- `01`: enable backdoor and use the simulated chamber status

Simulated chamber status command:

```text
DA 57 03 0A YY
```

- `0A`: simulated chamber status control
- `00`: simulated chamber status 0
- `01`: simulated chamber status 1
- `02`: simulated chamber status 2
- `03`: simulated chamber status 3

Command list:

```text
DA 57 03 FF 00  Disable backdoor and return to real GPIO pin detection
DA 57 03 FF 01  Enable backdoor and use the simulated chamber status

DA 57 03 0A 00  Set simulated chamber status to 0
DA 57 03 0A 01  Set simulated chamber status to 1
DA 57 03 0A 02  Set simulated chamber status to 2
DA 57 03 0A 03  Set simulated chamber status to 3
```

Example flow:

```text
DA 57 03 FF 01
DA 57 03 0A 02
```

After this, a CDC read of `MSGaddress_TYPE_20` returns chamber status `0x02`. To restore hardware pin detection:

```text
DA 57 03 FF 00
```
