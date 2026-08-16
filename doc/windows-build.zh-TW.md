# Windows 建置、picotool 與 UF2 操作手冊

本文件說明如何在 Windows 上建置 Pico-app，以及缺少 `picotool` 時，
如何從原始碼建置並安裝 picotool 2.1.1。以下路徑是本機目前使用的
實際路徑：

```text
Pico-app：         D:\yqgithub\Pico-app
Pico SDK 2.1.1：  D:\YQRepo\pico\pico-sdk
picotool source： D:\YQRepo\pico\picotool
picotool install：D:\YQRepo\pico\picotool-install
```

## 1. 各工具的用途

- Pico SDK：提供 GPIO、UART、Flash、USB 等函式庫及 CMake 規則。
- ARM GCC：把 Pico-app 的 C source 編譯及連結成 ARM firmware。
- CMake：產生建置設定。
- Ninja：執行 Pico-app 的實際編譯工作。
- picotool：Pico SDK 2.x 用來處理 ELF、BIN、UF2、hash 與簽章的 host
  工具。本專案需要它把 `host.elf` 後處理成 `host.uf2`。

GitHub 不負責編譯 Pico-app。若本機找不到相容的 picotool，Pico SDK
才會嘗試從 Raspberry Pi 的 GitHub repository 下載 picotool source，
然後在本機編譯。無法連上 GitHub 時，可能已經成功產生 ELF，卻仍因
缺少 picotool 而無法完成 UF2。

## 2. PowerShell 與 CMD 指令差異

看到以下提示代表 PowerShell：

```text
PS C:\...>
```

切換目錄及設定環境變數：

```powershell
Set-Location -LiteralPath "D:\YQRepo\pico\picotool"
$env:PICO_SDK_PATH = "D:\YQRepo\pico\pico-sdk"
```

看到以下提示代表 CMD／Developer Command Prompt：

```text
C:\...>
```

CMD 不認得 `Set-Location` 或 `$env:...`。跨磁碟切換與設定環境變數要用：

```bat
cd /d D:\YQRepo\pico\picotool
set PICO_SDK_PATH=D:\YQRepo\pico\pico-sdk
```

在 PowerShell 中輸入多行命令時，每個未結束行的最後一個字元必須是
反引號 `` ` ``，而且後面不能有空格。最安全的方法是將 CMake 命令以
完整單行貼上，只在最後按一次 Enter。終端機因寬度不足而自動折行不
影響命令；不要自行在參數中間按 Enter。

## 3. Pico-app 的 Builder.bat

`Builder.bat` 是建置入口，它把板型與模式交給
`tools/build_firmware.ps1`。該腳本會：

1. 檢查 `PICO_SDK_PATH` 與 Ninja。
2. 依板型選擇 Cortex-M0+ 或 Cortex-M33 ARM compiler。
3. 執行 CMake configure。
4. 執行 Ninja build。
5. 檢查 ELF，並在一般模式檢查 UF2。

四種板型：

```powershell
.\Builder.bat pico
.\Builder.bat pico_w
.\Builder.bat waveshare_rp2040_zero
.\Builder.bat pico2
```

未指定板型時，預設為 Pico：

```powershell
.\Builder.bat
```

### 一般模式

```powershell
.\Builder.bat pico
```

一般模式需要相容的 picotool，預期產生：

```text
build\pico\host.elf
build\pico\host.bin
build\pico\host.hex
build\pico\host.uf2
```

### elf-only 模式

```powershell
.\Builder.bat pico elf-only
```

此模式設定 `PICO_NO_PICOTOOL=1`，只驗證 C source 能否完成 compiler
及 linker，不要求 picotool，因此不會產生 `host.uf2`。通常仍會產生
ELF、BIN 與 HEX。

## 4. 檢查現有 picotool

先確認是否已有可執行檔：

```powershell
Get-Command picotool -ErrorAction SilentlyContinue
Get-ChildItem "D:\YQRepo\pico" -Recurse -Filter picotool.exe
```

只有 `D:\YQRepo\pico\picotool` source directory 不代表已安裝；必須
能找到 `picotool.exe`，且版本符合 Pico SDK 2.1.1：

```powershell
& "<picotool.exe 的完整路徑>" version
```

## 5. 將 picotool source 固定為 2.1.1

進入 repo 後確認沒有未提交變更，再抓取 tag 並切換：

```powershell
Set-Location -LiteralPath "D:\YQRepo\pico\picotool"
git status --short --branch
git fetch origin --tags --prune
git switch --detach 2.1.1
git describe --tags --exact-match
```

最後一行應輸出：

```text
2.1.1
```

停在 tag 時顯示 detached HEAD 是正常的固定版本狀態。不要在 repo 有
未提交變更時直接切換版本。

## 6. 建置 picotool.exe

### 6.1 啟動 MSVC 開發環境

從 Windows 開始功能表開啟：

```text
Developer PowerShell for VS 2022
```

切換到 picotool repo，並由這個終端機啟動 VS Code：

```powershell
Set-Location -LiteralPath "D:\YQRepo\pico\picotool"
code D:\YQRepo\pico\picotool
```

如果使用的是 Developer Command Prompt（CMD），改用：

```bat
cd /d D:\YQRepo\pico\picotool
code D:\YQRepo\pico\picotool
```

在新開啟的 VS Code Terminal 確認 MSVC 可用：

```text
cl
```

應顯示 Microsoft C/C++ compiler 版本，而不是「不是內部或外部命令」。

### 6.2 設定 SDK

PowerShell：

```powershell
$env:PICO_SDK_PATH = "D:\YQRepo\pico\pico-sdk"
Test-Path "$env:PICO_SDK_PATH\pico_sdk_version.cmake"
```

測試應回傳 `True`。

CMD：

```bat
set PICO_SDK_PATH=D:\YQRepo\pico\pico-sdk
if exist "%PICO_SDK_PATH%\pico_sdk_version.cmake" (echo SDK OK) else (echo SDK NOT FOUND)
```

### 6.3 CMake configure

本專案只需要 picotool 的檔案／UF2 處理能力，所以使用
`PICOTOOL_NO_LIBUSB=1`，不建置直接控制 USB Pico 的功能，也不需要另
裝 libusb。

請將以下內容當成完整單行貼到 PowerShell，只在最後按 Enter：

```powershell
cmake -S . -B build -G "NMake Makefiles" -DPICOTOOL_NO_LIBUSB=1 -DPICOTOOL_FLAT_INSTALL=1 "-DCMAKE_INSTALL_PREFIX=D:\YQRepo\pico\picotool-install" "-DCMAKE_POLICY_VERSION_MINIMUM=3.5"
```

`CMAKE_POLICY_VERSION_MINIMUM=3.5` 是因為目前 CMake 4.x 已移除部分舊
CMake compatibility，而 Pico SDK 2.1.1 內的 mbedTLS 尚使用舊最低版本
宣告。這個參數允許它繼續 configure。

成功標誌：

```text
-- Configuring done
-- Generating done
-- Build files have been written to: D:/YQRepo/pico/picotool/build
```

`CMake Deprecation Warning` 或 `PICOTOOL_NO_LIBUSB is set` 在這個流程中
不是失敗；出現 `CMake Error` 才需要停止。

### 6.4 編譯

```powershell
cmake --build build
```

尋找並確認版本：

```powershell
Get-ChildItem .\build -Recurse -Filter picotool.exe
& "D:\YQRepo\pico\picotool\build\picotool.exe" version
```

預期類似：

```text
picotool v2.1.1 (Windows, MSVC-..., Debug)
```

Debug 或 Release 都能完成本專案所需的 ELF-to-UF2 工作。

### 6.5 安裝

```powershell
cmake --install build
Get-ChildItem "D:\YQRepo\pico\picotool-install" -Recurse -Filter picotool.exe
```

預期安裝位置：

```text
D:\YQRepo\pico\picotool-install\picotool\picotool.exe
```

確認：

```powershell
& "D:\YQRepo\pico\picotool-install\picotool\picotool.exe" version
```

## 7. 使用已安裝的 picotool 建置 Pico-app UF2

在同一個 PowerShell 終端機執行：

```powershell
Set-Location -LiteralPath "D:\yqgithub\Pico-app"
$env:PICO_SDK_PATH = "D:\YQRepo\pico\pico-sdk"
$env:picotool_DIR = "D:\YQRepo\pico\picotool-install\picotool"
Test-Path "$env:picotool_DIR\picotool.exe"
.\Builder.bat pico
Get-Item .\build\pico\host.uf2
```

`Test-Path` 應回傳 `True`，最後一行應找到 `host.uf2`。

CMD 等效指令：

```bat
cd /d D:\yqgithub\Pico-app
set PICO_SDK_PATH=D:\YQRepo\pico\pico-sdk
set picotool_DIR=D:\YQRepo\pico\picotool-install\picotool
Builder.bat pico
dir build\pico\host.uf2
```

這兩個環境變數預設只對目前終端機有效；關閉終端機後，下次需要重新
設定。

## 8. 燒錄 UF2

1. 按住 Pico 的 BOOTSEL。
2. 將 Pico 透過 USB 接到電腦。
3. 放開 BOOTSEL。
4. 等待 `RPI-RP2` USB 磁碟出現。
5. 將 `build\<board>\host.uf2` 複製到該磁碟。

## 9. 常見錯誤

### `Set-Location` 不是內部或外部命令

目前終端機是 CMD，不是 PowerShell。使用：

```bat
cd /d D:\YQRepo\pico\picotool
```

### `-DPICOTOOL_NO_LIBUSB=1` 不是 cmdlet

PowerShell 中途按了 Enter，導致 CMake 參數被當成新命令。改用完整單行，
或在每個未結束行最後加反引號。

### `Unexpected token '-DCMAKE'`

引號內的路徑或參數被手動斷行，例如把 `picotool-install` 拆成兩行。
重新貼上完整單行，不要自行換行。

### `Invalid CMAKE_POLICY_VERSION_MINIMUM value "3"`

PowerShell 把 `3.5` 拆開。確保完整參數有雙引號：

```powershell
"-DCMAKE_POLICY_VERSION_MINIMUM=3.5"
```

### 找不到相容 picotool，接著嘗試 GitHub

代表 Pico SDK 沒找到已安裝的 picotool package。確認：

```powershell
$env:picotool_DIR = "D:\YQRepo\pico\picotool-install\picotool"
Test-Path "$env:picotool_DIR\picotool.exe"
```

再重新執行 `Builder.bat pico`。

### 只有 ELF/BIN/HEX，沒有 UF2

- 若使用 `elf-only`，這是預期行為。
- 若使用一般模式，檢查 picotool 版本、`picotool_DIR` 與 configure error。

### `CMake Deprecation Warning`

Warning 不等於失敗。只要最後出現 `Configuring done`、`Generating done`
且沒有 `CMake Error`，即可繼續 build。
