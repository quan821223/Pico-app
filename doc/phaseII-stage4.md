# Phase II 第四階段：Board Profile 與建置矩陣

## 完成內容

硬體腳位與板型能力已從 `main.c`、`pico_app_io.c` 的 hard-coded constants 移到純 C `board_profile`：

```text
src/board/board_profile.c/.h
```

目前定義四個 profiles：

| ID | CMake `PICO_BOARD` | MCU | Flash | UART TX/RX | 指示燈 |
|---|---|---|---:|---|---|
| 0 | `pico` | RP2040 | 2 MiB | GP16/GP17 | GPIO GP25 |
| 1 | `pico_w` | RP2040 | 2 MiB | GP16/GP17 | CYW43，暫停驅動 |
| 2 | `waveshare_rp2040_zero` | RP2040 | 2 MiB | GP0/GP1 | WS2812 GP16，暫停驅動 |
| 3 | `pico2` | RP2350 | 4 MiB | GP16/GP17 | GPIO GP25 |

Chamber GP2/GP3、address GP18/GP19、Touch GP20/GP21 目前依已確認需求在四個 profiles 保持一致。這些值不再散落於 application code。

## 為什麼指示燈使用 capability type

Pico/Pico 2 的 GP25 是一般 GPIO；Pico W LED 透過 CYW43；RP2040-Zero 是需要精確 waveform 的 WS2812。三者不能使用相同 `gpio_put()`。

`board_indicator_kind_t` 明確區分 GPIO、CYW43、WS2812 與 unavailable。Stage 4 只啟用一般 GPIO driver；CYW43 與 WS2812 安全 no-op，避免錯誤電氣操作。若未來需要燈號，再各自增加 driver，不污染 `main.c`。

## CMake 防呆

- 只接受四個已支援的 `PICO_BOARD`，其他值 configure 時立即失敗。
- 每個 board 映射到明確 `APP_ACTIVE_BOARD_ID`。
- 強制 Pico SDK version 必須等於 2.1.1，避免每台電腦使用不同 SDK 卻產生看似成功的韌體。
- RP2040 使用 `pico_arm_cortex_m0plus_gcc`；Pico 2 使用 `pico_arm_cortex_m33_gcc`。

## Windows 建置方式

正常產生 ELF/UF2：

```bat
Builder.bat pico
Builder.bat pico_w
Builder.bat waveshare_rp2040_zero
Builder.bat pico2
```

沒有相容 picotool、只做 compiler/linker 驗證：

```bat
Builder.bat pico elf-only
```

真正邏輯位於 `tools/build_firmware.ps1`，兩個 `.bat` 只是相容入口。每個 board 使用獨立的 `build/<board>/`，切換板型不會重用錯誤的 CMake cache。可透過 `-PicoSdkPath` 明確指定 SDK；未指定時才讀取環境變數，兩種方式都會受 2.1.1 version gate 檢查。

## 驗證結果

四個 profiles 均以 Pico SDK 2.1.1 完成 ARM ELF 建置：

| Profile | text | BSS | Active ID |
|---|---:|---:|---:|
| Pico | 28,308 | 4,188 | 0 |
| Pico W | 28,308 | 4,188 | 1 |
| RP2040-Zero | 28,308 | 4,188 | 2 |
| Pico 2 | 26,564 | 3,540 | 3 |

Native tests 會檢查 profiles 有效、SDK board name 唯一、功能腳位不互相衝突，以及 Zero/Pico W/Pico 2 的差異。

## 尚未完成

- Profile 保存到 Flash、Save Configuration 與 factory reset。
- 開機硬體自動偵測；沒有可靠證據時仍以 build/saved profile 為準。
- Pico W CYW43 LED driver。
- RP2040-Zero WS2812 LED driver。
- 三款現有板的實體 GPIO/UART 驗證；Pico 2 目前只有編譯驗證。
- 正式 UF2 建置需要安裝與 Pico SDK 2.1.1 相容的 picotool。

