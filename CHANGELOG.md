# CHANGELOG - Pico-app

所有重要變更都會記錄在此檔案。

本專案版本紀錄參考 Semantic Versioning 格式；韌體協定變更需同步更新 `README.md`、`doc/current-protocol.md` 與 characterization vectors。

## [Unreleased]

**✨ 新增**

- （尚未釋出的功能請記在此處）

## [0.2.0] - 2026-09-04

What’s changed since 0.1.x

**✨ 新增**

- 新增 `DA 52 0B 00 00` 微動開關狀態讀取指令。
- 新增 `DA 57 0B 00 00` 微動開關讀取計數器重置指令，回應 `C3 0D 0A`。
- 保留 `DA 57 03 0B 00` 作為微動開關重置相容指令。
- 新增 `FA 52 01 0B 00` 單一 fixture angle 讀取回應。
- 新增 `FA 52 00 0B 00` 全 fixture angle 讀取回應。

**🔧 變更**

- `FA 52 01 1B 00` content response 現在會回填 request device byte。
- `DA 57` touch/chamber write 行為重新限制在 `device == 0x03`，避免其他 DA device 誤觸發 touch 或 chamber effect。
- 微動開關狀態在 reset 後前九次 `DA 52 0B 00 00` 回傳 `00`，第十次起回傳 `01`，直到下一次 reset。
- fixture angle 回應共用微動開關讀取計數器，threshold 前回傳 `00`，threshold 後回傳 `B3`。

**🧪 測試**

- 新增 `DA 57 0B 00 00` reset ACK 與 effect 單元測試。
- 新增 `DA 57 03 0B 00` 相容 reset ACK 與 effect 單元測試。
- 新增 `DA 52 0B 00 00` 讀取計數器 threshold 行為單元測試。
- 更新 characterization vectors，記錄新的 micro switch / fixture angle 指令與 content response 變更。

**📝 文件**

- 整理 README Windows 建置流程：工具檢查、Ninja 安裝、Pico SDK、picotool、UF2 產生與燒錄順序。
- 補充 `winget install Ninja-build.Ninja`，避免 `Ninja is required and was not found in PATH.`。
- 補充 VS 2019 / VS 2022 CMake generator 對應與 `NMake Makefiles` Native Tools Prompt 使用方式。
- 補充 CMake build directory 不可混用 generator 的說明。
- 新增 micro switch / fixture angle 指令操作說明到 `README.md` 與 `doc/current-protocol.md`。