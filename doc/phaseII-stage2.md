# Phase II 第二階段紀錄：純 C 串流 Parser

## 完成範圍

第二階段只抽離五位元組串流 parser，不搬動 FA／DA command dispatch、GPIO effects 或 response tables。USB CDC 收到資料後依序經過：

```text
TinyUSB callback
  -> receive_data()
  -> protocol_stream_parser_feed()
  -> 完整 5-byte frame callback
  -> 原有 process_message()
```

因此既有 command 對應與回應內容仍由原程式負責。

## Parser 行為

- 沒看到 `FA`／`DA` 前忽略雜訊。
- 收到 header 後開始收集五個 bytes。
- 收集中再次看到 `FA`／`DA`，放棄 partial frame，從新 header 開始。
- 一次輸入可以產生多個完整 frames。
- frame 可以分散在多次 USB callbacks 中。
- timeout 使用 unsigned subtraction，能正確跨越 32-bit millisecond counter wraparound。
- parser 不配置 heap，也不包含 Pico SDK header。

## Timeout 邊界

Stage 2 當時只完成 `protocol_stream_parser_expire()` API；Stage 5 已由
production main loop 呼叫此 API，並在 partial frame 逾時後立即回覆
`EC 00 00 00 00 0D 0A`。純雜訊 batch 的單次錯誤回覆仍未實作。

## 測試方式

```powershell
cmake -S tests/unit -B build-host-tests -G Ninja -DCMAKE_C_COMPILER=C:/MinGW/bin/gcc.exe
cmake --build build-host-tests
ctest --test-dir build-host-tests --output-on-failure
```

測試以 `-Wall -Wextra -Wpedantic -Werror` 編譯。CTest 目前有一個 parser test executable，內含八個獨立場景。

RP2040 離線編譯驗證：

```powershell
cmake -S . -B build-phase2 -G Ninja `
  -DPICO_BOARD=pico `
  -DPICO_COMPILER=pico_arm_cortex_m0plus_gcc `
  -DPICO_NO_PICOTOOL=1
cmake --build build-phase2
```

`PICO_NO_PICOTOOL=1` 只用於無網路的編譯驗證，因此產生 ELF 而不產生 UF2。正式開發環境仍需要與 Pico SDK 2.1.1 相容的 picotool。

## 下一個安全切點

下一階段應抽離 command decoder/dispatcher 與 response encoder，使 characterization JSON 的 request/response vectors 能直接執行 production logic；GPIO 行為以 mock interface 驗證，不由 unit tests 操作真實硬體。
