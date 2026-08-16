# Pico-app

This is a private repo project for Raspberry Pi Pico application.

## Project status

Phase II stages 2 through 5 are present in the current working tree. The latest
stage adds runtime board/timing configuration, a separate `0xCF` configuration
protocol, incomplete-frame timeout handling, and two-slot CRC-protected Flash
persistence. See [doc/README.md](doc/README.md) for the design and stage records,
and [HANDOFF.md](HANDOFF.md) for the verified state and remaining work.

The bootloader, final A/B Flash layout, factory reset, hardware-in-the-loop
validation, and sinusoidal temperature simulation are not complete.

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

## Recommended build flow: Builder.bat

完整的繁體中文逐步操作、picotool 2.1.1 安裝、UF2 產生與常見錯誤，請
參閱 [Windows 建置、picotool 與 UF2 操作手冊](doc/windows-build.zh-TW.md)。

Open PowerShell in the project root and set the Pico SDK path for the current
terminal:

```powershell
cd D:\yqgithub\Pico-app
$env:PICO_SDK_PATH = "D:\YQRepo\pico\pico-sdk"
```

Then select the board to build:

```powershell
.\Builder.bat pico
.\Builder.bat pico_w
.\Builder.bat waveshare_rp2040_zero
.\Builder.bat pico2
```

Running `.\Builder.bat` without a board name defaults to `pico`.

`Builder.bat` is a small command-line entry point. It passes the board and mode
to `tools/build_firmware.ps1`, which checks the SDK and Ninja, selects the correct
ARM compiler, configures CMake, builds the firmware, and checks the output. Each
board has an independent output directory under `build/<board>/`.

### Normal mode: produce a UF2

Use normal mode when you need a file that can be copied to the Pico BOOTSEL
drive:

```powershell
.\Builder.bat pico
```

Expected outputs include:

```text
build/pico/host.elf
build/pico/host.bin
build/pico/host.hex
build/pico/host.uf2
```

This mode requires a Pico SDK 2.1.1-compatible `picotool`.

### elf-only mode: compile without a UF2

Use this mode to verify that the C source compiles and links when picotool is
not available:

```powershell
.\Builder.bat pico elf-only
```

It produces ELF/BIN/HEX outputs but deliberately does not produce `host.uf2`.
The ELF contains the linked firmware and debugging symbols; it proves that the
compiler and linker completed, but it is not the convenient BOOTSEL copy format.

### Why producing a UF2 can access GitHub

GitHub is not needed to compile this project's C source. The connection happens
in a later output-conversion step:

1. ARM GCC compiles and links the project into `host.elf`.
2. Pico SDK asks `picotool` to post-process that firmware into `host.uf2`.
3. If a compatible installed picotool cannot be found, Pico SDK's CMake scripts
   try to download the picotool source from its Raspberry Pi GitHub repository.
4. CMake builds that helper locally and then uses it to create the UF2.

On this machine, no compatible installed picotool was found and the automatic
GitHub download could not connect to port 443. Therefore the C firmware can be
built in `elf-only` mode, but normal mode cannot currently finish `host.uf2`.
Seeing GitHub in the error does not mean the application source is hosted or
compiled on GitHub; CMake is only trying to obtain a missing build tool.

After normal mode succeeds, locate the UF2 with:

```powershell
Get-ChildItem .\build\pico\host.uf2
```

To flash it, hold the board's BOOTSEL button while connecting USB, release the
button when the `RPI-RP2` drive appears, and copy `host.uf2` to that drive.

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

## Host tests

Configure and run the native unit and characterization tests with:

```powershell
cmake -S tests/unit -B build-host-tests
cmake --build build-host-tests
ctest --test-dir build-host-tests --output-on-failure
```

The suite covers the stream parser, application protocol, protocol service,
board profiles, runtime configuration journal, and byte-exact characterization
contract.

## Runtime configuration

Stage 5 reserves the `0xCF` control namespace for runtime configuration. Board
profile, FA/DA response delay, and incomplete-frame timeout can be changed in
RAM and explicitly saved to a two-slot Flash journal. Board changes take effect
after restart; timing changes take effect immediately. The request/response
bytes and Flash record format are documented in
[doc/phaseII-stage5.md](doc/phaseII-stage5.md).

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
