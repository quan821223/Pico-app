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

Windows 建置與燒錄建議先確認工具，再執行 `Builder.bat`。完整的繁體中文逐步操作、picotool 2.1.1 安裝、UF2 產生與常見錯誤，也可參閱 [Windows 建置、picotool 與 UF2 操作手冊](doc/windows-build.zh-TW.md)。

需要的工具：

- CMake
- Ninja，`Builder.bat pico` 會檢查它是否在 `PATH`
- GNU Arm Embedded Toolchain，例如 `arm-none-eabi-gcc`
- Raspberry Pi Pico SDK，透過 `PICO_SDK_PATH` 指定
- Pico SDK 2.1.1-compatible `picotool`，正常 UF2 build 需要它
- Visual Studio 2019 或 2022，只有在需要自己建置 picotool 時才需要

在 PowerShell 先檢查：

```powershell
cmake --version
ninja --version
arm-none-eabi-gcc --version
$env:PICO_SDK_PATH
Test-Path "$env:PICO_SDK_PATH\external\pico_sdk_import.cmake"
```

預期結果：

- `cmake --version` 顯示 CMake 版本。
- `ninja --version` 顯示 Ninja 版本。
- `arm-none-eabi-gcc --version` 顯示 GNU Arm Embedded Toolchain 版本。
- `$env:PICO_SDK_PATH` 指向 Pico SDK 目錄。
- `Test-Path ...pico_sdk_import.cmake` 回傳 `True`。

若缺少 Ninja，先安裝：

```powershell
winget install Ninja-build.Ninja
```

安裝後關閉並重新開啟 PowerShell，再確認：

```powershell
ninja --version
where.exe ninja
```

若缺少 ARM toolchain，將 `bin` 目錄加到目前 terminal 的 `PATH`：

```powershell
$env:Path = "C:\Program Files (x86)\GNU Arm Embedded Toolchain\10 2021.10\bin;" + $env:Path
arm-none-eabi-gcc --version
```

設定 Pico SDK 路徑。此專案常用路徑如下，請依實際電腦調整：

```powershell
$env:PICO_SDK_PATH = "D:\YQRepos\pico\pico-sdk"
Test-Path "$env:PICO_SDK_PATH\external\pico_sdk_import.cmake"
```

如果 Git 因 `dubious ownership` 擋住 Pico SDK，執行：

```powershell
git config --global --add safe.directory D:\YQRepos\pico\pico-sdk
```

## Recommended Windows build flow

### 1. Open the project root

```powershell
Set-Location -LiteralPath "D:\YQHubRepo\Pico-app"
$env:PICO_SDK_PATH = "D:\YQRepos\pico\pico-sdk"
```

### 2. Verify required tools

```powershell
cmake --version
ninja --version
arm-none-eabi-gcc --version
Test-Path "$env:PICO_SDK_PATH\external\pico_sdk_import.cmake"
```

`Builder.bat pico` requires Ninja. If it fails with `Ninja is required and was not found in PATH.`, install Ninja with `winget install Ninja-build.Ninja`, reopen PowerShell, and run `ninja --version` again.

### 3. Prepare picotool when building UF2

Normal mode produces `host.uf2`, so Pico SDK needs a compatible `picotool`.

Check whether picotool is already installed:

```powershell
$env:picotool_DIR = "D:\YQRepos\pico\picotool-install\picotool"
Test-Path "$env:picotool_DIR\picotool.exe"
```

If this returns `True`, verify the executable:

```powershell
& "$env:picotool_DIR\picotool.exe" version
```

If this returns `False`, first check whether `picotool.exe` already exists under the source tree:

```powershell
Get-ChildItem "D:\YQRepos\pico\picotool" -Recurse -Filter picotool.exe
```

If no executable is found, build and install picotool from source.

### 4. Build firmware

Select the board to build:

```powershell
.\Builder.bat pico
.\Builder.bat pico_w
.\Builder.bat waveshare_rp2040_zero
.\Builder.bat pico2
```

Running `.\Builder.bat` without a board name defaults to `pico`.

Expected normal-mode outputs include:

```text
build/pico/host.elf
build/pico/host.bin
build/pico/host.hex
build/pico/host.uf2
```

After normal mode succeeds, locate the UF2 with:

```powershell
Get-ChildItem .\build\pico\host.uf2
```

### 5. Flash firmware

Hold the board's BOOTSEL button while connecting USB, release the button when the `RPI-RP2` drive appears, and copy `host.uf2` to that drive.

## Build picotool from source

Use this only when normal UF2 build cannot find a compatible `picotool`.

Important CMake rule: a build directory is tied to the generator that created it. Do not reuse a CMake build directory with a different generator. For example, if `build` was created with `Visual Studio 17 2022`, do not reuse that same `build` directory for `NMake Makefiles` or VS 2019. Use a separate directory such as `build-vs2019`, `build-vs2022`, or `build-nmake`.

Generator choices:

- Visual Studio 2019: `-G "Visual Studio 16 2019" -A x64`
- Visual Studio 2022: `-G "Visual Studio 17 2022" -A x64`
- NMake: `-G "NMake Makefiles"`

When using `NMake Makefiles`, run CMake from the matching Native Tools prompt. A normal PowerShell may fail with `nmake` not found.

- VS 2019: `x64 Native Tools Command Prompt for VS 2019`
- VS 2022: `x64 Native Tools Command Prompt for VS 2022`

NMake example:

```powershell
Set-Location -LiteralPath "D:\YQRepos\pico\picotool"
git fetch --tags
git checkout 2.1.1

$env:PICO_SDK_PATH = "D:\YQRepos\pico\pico-sdk"
cmake -S . -B build-nmake -G "NMake Makefiles" -DPICOTOOL_NO_LIBUSB=1 -DPICOTOOL_FLAT_INSTALL=1 "-DCMAKE_INSTALL_PREFIX=D:\YQRepos\pico\picotool-install" "-DCMAKE_POLICY_VERSION_MINIMUM=3.5"
cmake --build build-nmake
cmake --install build-nmake

$env:picotool_DIR = "D:\YQRepos\pico\picotool-install\picotool"
Test-Path "$env:picotool_DIR\picotool.exe"
& "$env:picotool_DIR\picotool.exe" version
```

Visual Studio generator examples:

```powershell
Set-Location -LiteralPath "D:\YQRepos\pico\picotool"
git fetch --tags
git checkout 2.1.1
$env:PICO_SDK_PATH = "D:\YQRepos\pico\pico-sdk"

# VS 2019
cmake -S . -B build-vs2019 -G "Visual Studio 16 2019" -A x64 -DPICOTOOL_NO_LIBUSB=1 -DPICOTOOL_FLAT_INSTALL=1 "-DCMAKE_INSTALL_PREFIX=D:\YQRepos\pico\picotool-install" "-DCMAKE_POLICY_VERSION_MINIMUM=3.5"
cmake --build build-vs2019 --config Release
cmake --install build-vs2019 --config Release

# VS 2022
cmake -S . -B build-vs2022 -G "Visual Studio 17 2022" -A x64 -DPICOTOOL_NO_LIBUSB=1 -DPICOTOOL_FLAT_INSTALL=1 "-DCMAKE_INSTALL_PREFIX=D:\YQRepos\pico\picotool-install" "-DCMAKE_POLICY_VERSION_MINIMUM=3.5"
cmake --build build-vs2022 --config Release
cmake --install build-vs2022 --config Release
```

After `Test-Path "$env:picotool_DIR\picotool.exe"` returns `True`, rerun the firmware build:

```powershell
Set-Location -LiteralPath "D:\YQHubRepo\Pico-app"
.\Builder.bat pico
```

## elf-only mode: compile without UF2

Use this mode to verify that the C source compiles and links when picotool is not available:

```powershell
.\Builder.bat pico elf-only
```

It produces ELF/BIN/HEX outputs but deliberately does not produce `host.uf2`. The ELF contains the linked firmware and debugging symbols; it proves that the compiler and linker completed, but it is not the convenient BOOTSEL copy format.

## Why producing UF2 can access GitHub

GitHub is not needed to compile this project's C source. The connection happens in a later output-conversion step:

1. ARM GCC compiles and links the project into `host.elf`.
2. Pico SDK asks `picotool` to post-process that firmware into `host.uf2`.
3. If a compatible installed picotool cannot be found, Pico SDK's CMake scripts try to download the picotool source from its Raspberry Pi GitHub repository.
4. CMake builds that helper locally and then uses it to create the UF2.

If the automatic GitHub download cannot connect to port 443, install/build picotool manually with the steps above, then rerun normal mode.

## Manual build alternatives

`Builder.bat` is the recommended entry point. It passes the board and mode to `tools/build_firmware.ps1`, which checks the SDK and Ninja, selects the correct ARM compiler, configures CMake, builds the firmware, and checks the output. Each board has an independent output directory under `build/<board>/`.

Manual Ninja build:

```powershell
Set-Location -LiteralPath "D:\YQHubRepo\Pico-app"
$env:PICO_SDK_PATH = "D:\YQRepos\pico\pico-sdk"
$env:Path = "C:\Program Files (x86)\GNU Arm Embedded Toolchain\10 2021.10\bin;" + $env:Path

Remove-Item build -Recurse -Force -ErrorAction SilentlyContinue
cmake -S . -B build -G Ninja -DPICO_COMPILER=pico_arm_cortex_m0plus_gcc
cmake --build build
```

Manual NMake build, from a Visual Studio developer terminal:

```powershell
Set-Location -LiteralPath "D:\YQHubRepo\Pico-app"
$env:PICO_SDK_PATH = "D:\YQRepos\pico\pico-sdk"
$env:Path = "C:\Program Files (x86)\GNU Arm Embedded Toolchain\10 2021.10\bin;" + $env:Path

Remove-Item build -Recurse -Force -ErrorAction SilentlyContinue
cmake -S . -B build -G "NMake Makefiles" -DPICO_COMPILER=pico_arm_cortex_m0plus_gcc
cmake --build build
```

If CMake output says `The C compiler identification is MSVC`, the build directory is configured incorrectly for Pico. Delete `build`, make sure `arm-none-eabi-gcc --version` works, then configure again. Pico should use ARM GCC, not MSVC `cl.exe`.

## Common environment problems

### Ninja is missing

Error example:

```text
Ninja is required and was not found in PATH.
```

Cause: `Builder.bat pico` requires Ninja, but `ninja.exe` is not installed or not in `PATH`.

Fix:

```powershell
winget install Ninja-build.Ninja
```

Close and reopen PowerShell, then check:

```powershell
ninja --version
where.exe ninja
```

### NMake is missing

Error example:

```text
Running

 'nmake' '-?'

failed with:

 no such file or directory
```

Cause: `NMake Makefiles` was used from a terminal that did not load the Visual Studio build environment.

Fix: open the matching `x64 Native Tools Command Prompt` for VS 2019 or VS 2022, then run:

```powershell
nmake /?
```

### Build directory has the wrong generator

Error example:

```text
generator : NMake Makefiles
Does not match the generator used previously: Visual Studio 17 2022
```

Cause: the same CMake build directory was reused with a different generator.

Fix: use a different build directory, such as `build-nmake`, `build-vs2019`, or `build-vs2022`. Only delete a build directory when you are sure it contains disposable CMake output.

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
$env:PICO_SDK_PATH = "D:\YQRepos\pico\pico-sdk"
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

## Micro switch and fixture angle commands

These commands use the normal USB CDC protocol path, not the UART chamber backdoor. Send each request as raw HEX bytes with the standard five-byte layout:

```text
[header] [operation] [device] [category] [parameter]
```

### Read micro switch status

```text
DA 52 0B 00 00
```

Response before the read counter reaches the threshold:

```text
DA 0B 08 00 00 00 00 00 00 0D 0A
```

Response after the threshold is reached:

```text
DA 0B 08 01 01 01 01 01 01 0D 0A
```

The firmware currently returns `00` for the first nine reads after reset. From the tenth read onward, bytes 3 through 8 return `01` and stay `01` until the reset command is received.

### Reset micro switch read counter

Recommended reset command:

```text
DA 57 0B 00 00
```

Compatibility reset command:

```text
DA 57 03 0B 00
```

Both reset commands return ACK:

```text
C3 0D 0A
```

After reset, the next `DA 52 0B 00 00` response returns all switch status bytes to `00`.

### Read fixture angle value

Single fixture angle command:

```text
FA 52 01 0B 00
```

Initial response:

```text
FA 01 03 00 0D 0A
```

All fixture angle command:

```text
FA 52 00 0B 00
```

Initial response:

```text
FA 01 05 00 00 00 0D 0A
```

The fixture angle value is derived from the same micro switch read counter. It returns `00` while the counter is within the initial range and `B3` after the counter crosses the threshold.

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
