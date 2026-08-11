# Phase II 重構設計說明

本文件以繁體中文說明本次重構的核心決策。詳細協定向量、待決事項與測試規則，請搭配同目錄其他文件閱讀。文件內標示「規劃」的功能尚未實作。

## 1. 為什麼要分層

現有程式把 USB、UART、封包解析、GPIO、延遲及回應內容放在少數檔案中，造成任何修改都可能影響現場協定。新架構會分成：

1. `protocol`：只處理 byte stream、5-byte 封包、timeout 與錯誤判定。
2. `application`：決定 FA／DA 指令應回覆什麼，以及要觸發哪個邏輯 IO。
3. `board`：把「狀態燈」「Chamber output」等邏輯名稱轉成各板 GPIO。
4. `transport`：USB CDC 與 UART mirror，不能包含產品邏輯。
5. `config`：保存板型、延遲、timeout 與更新資訊。
6. `bootloader`：接收韌體、CRC32、A/B slot、版本及回復策略。
7. `platform`：唯一允許直接依賴 Pico SDK、Flash、GPIO、watchdog 的外層。

如此一來，協定與 A/B 狀態機可以直接在 PC 跑單元測試，而板子差異只需要修改 board profile。

## 2. 板型選擇

RP2040 的 Pico、Pico W、Waveshare RP2040-Zero共用主要 application 原始碼；Pico 2 是 RP2350，必須另外產生韌體，但共用架構與協定。

預計支援：

```powershell
Builder.bat pico
Builder.bat pico_w
Builder.bat waveshare_rp2040_zero
Builder.bat pico2
```

板型也能保存到 Flash，切換後重開機生效。安全預設是 Pico。優先順序會保留擴充空間：診斷用強制建置設定、有效的 Flash 設定、建置預設值，最後才是 Pico fallback；實作前仍會定義清楚 factory reset 與強制設定旗標。

Pico W 的 LED 需要 CYW43，現階段停用並記錄於文件。RP2040-Zero 的 WS2812 使用 GP16，與舊版 UART TX 衝突，因此規劃把 Zero 的 UART0 改為 GP0／GP1；正式套用前必須確認現場接線未占用。

## 3. 協定相容性

舊有 FA／DA 是固定 5 bytes。新架構仍維持：

- 既有合法 request／response bytes。
- 收到新的 FA／DA header 時，丟棄尚未完成的舊封包並重新同步。
- 一批資料含兩個合法封包時，分別解析與回覆。
- FA／DA 模擬回應預設延遲 50 ms，且可於建置時、執行時及保存後設定。
- 不完整封包預設等待 100 ms。

已核准的新版差異是：無效批次回覆 `EC 00 00 00 00 0D 0A`。錯誤回覆、Bootloader、UART ACK 及 IO 操作不套用 50 ms 模擬延遲。所有新增設定指令使用新的 control header，不占用 FA／DA。

## 4. Bootloader 更新流程

Raspberry Pi ROM 原生模式是 BOOTSEL／UF2 或 Picoboot，並不是 USB CDC。因本專案要求 Bootloader 繼續用 CDC 接收韌體，所以需要自製更新層。

規劃流程：

```text
上位機送 BC 57 01 00 01
→ Application 回 C3 0D 0A 並 flush USB
→ 立即重開進入 Bootloader
→ 以不同 PID、名稱 Pico Bootloader 重新枚舉
→ 接收 manifest 與韌體 chunks
→ 寫入非保底 slot
→ 檢查長度、板型、版本與 CRC32
→ 將新版標為 candidate 並重開
→ Application 自我檢查成功後確認新版
```

Bootloader 等待更新 60 秒；沒有開始更新就啟動有效 application。USB 線是否接著電腦不影響 application 成功判定。CRC32 只能檢查傳輸損壞，不能防止惡意修改；簽章與加密目前明確列為未實現安全項目。

## 5. A/B 並不是每次複製 A 到 B

Flash 中保留兩個等大的 application slot。Metadata 記錄哪一個是 known-good、哪一個是 candidate：

1. 更新時永遠不覆寫 known-good。
2. 新韌體寫入另一個 slot。
3. 新版最多嘗試啟動三次。
4. 新版自我檢查成功後，只更新 metadata，把它升格為 known-good。
5. 舊 known-good 之後成為下一次可覆寫的 candidate slot。
6. 三次失敗便把 candidate 標為無效，回到上一個 known-good。

不需要把整份 B 再複製到 A；只切換角色可以減少 Flash 磨耗、縮短時間，也降低複製途中斷電的風險。

目前 application `.bin` 約 32,444 bytes，而 RP2040 板載 Flash 通常是 2 MiB。暫定預算為 Bootloader 128 KiB、Slot A 896 KiB、Slot B 896 KiB、設定與 log 128 KiB。這只是容量 envelope；完成可建置的 Bootloader 後，會用 linker map、sector 對齊及 build-time overflow assertion 決定實際位址。

外接 Flash 代表現場三種板子都必須增加晶片／模組、供電、接線、driver 與故障處理，屬於硬體改版。因此只有內部 2 MiB 實測無法容納完整 A/B 時才評估，目前沒有必要。

## 6. 設定保存

board profile、回應延遲、封包 timeout、韌體版本、A/B 狀態、嘗試次數與更新結果需要跨斷電存在。USB serial number 由晶片 unique ID 推導，不需要另外頻繁寫 Flash。

設定先改在 RAM，收到明確的 Save Configuration 指令才寫 Flash。資料包含版本、長度、遞增序號與 CRC，並保留兩份可獨立 erase 的紀錄；更新其中一份斷電時，仍能讀回另一份有效紀錄。

Chamber backdoor 維持舊行為：開機停用，只在當次通電有效，不保存到 Flash。

## 7. 測試層級

- PC：parser、dispatch、byte-exact response、timeout、CRC、版本及 A/B 狀態機。
- 兩板 UART：驗證 UART backdoor；不能宣稱已驗證 USB CDC。
- 兩板 USB Host：測試真正 USB 枚舉、CDC、重新連線與 Bootloader 更新。
- SWD：研究 Raspberry Pi GPIO 與 Pico Debug Probe 兩種方式。RP2040-Zero 沒有標準 SWD 引腳時，以 USB/UART 黑箱驗收。
- GitHub Actions：跑 PC tests 與各 profile 編譯；實體硬體測試使用本機或 self-hosted runner。

Characterization tests 鎖定現況契約。第二階段已抽出純 C stream parser，native C tests 會直接編譯 production parser；FA／DA command dispatch 仍依賴 Pico SDK/GPIO，會在後續階段抽離並接上同一批 byte-exact vectors。

## 9. 第二階段實作狀態

- 已新增 `src/protocol/protocol_stream_parser.c/.h`，不依賴 Pico SDK。
- USB CDC 的 `receive_data()` 已改用新 parser，完整 frame 仍交給原本 `process_message()`，因此本階段沒有搬動既有回應邏輯。
- 已測試雜訊忽略、單一封包、跨批次組包、新 header 重新同步、單批兩封包、100 ms timeout、32-bit 計時 wraparound 與 reset。
- 100 ms timeout 能力已存在於 parser API，但 production 尚未回覆新的 `EC` 錯誤；錯誤輸出會等 command/response transport 分層後再接入，避免在本階段混入協定行為變更。
- 已以 ARM GCC/Pico SDK 2.1.1 成功產生 RP2040 `host.elf`。因離線驗證停用 picotool，該次驗證不產生 UF2。

## 10. 第三階段實作狀態

- 原本混合 parser、dispatch、response、GPIO 與 heap allocation 的 `tud_cdc_descript.*` 已移除。
- `app_protocol` 現在是純 C production command core，輸出固定 response 與 typed effect，不直接碰 GPIO。
- `protocol_service` 以 ports 連接 chamber input、IO effects 與延遲 response。
- `pico_app_io` 集中實作 Pico SDK GPIO/timer 與 RAM-only chamber backdoor。
- 42 組 JSON vectors 會透過 native runner 直接執行 production core，比對完整 response bytes。
- request path 已移除 `malloc/free`；66-byte Touch response 的 buffer overflow 已修復並登記於 change-control。

## 11. 第四階段實作狀態

- 已建立 Pico、Pico W、Waveshare RP2040-Zero 與 Pico 2 的純 C board profiles。
- 腳位、MCU family、Flash 容量與 LED capability 已離開 `main.c` 與 application logic。
- Zero UART 使用 GP0/GP1，GP16 保留給 WS2812；Pico W/Zero 非一般 GPIO LED 目前安全停用。
- `Builder.bat <board>` 使用每板獨立 cache，Pico SDK 非 2.1.1 會直接拒絕建置。
- 四個 profiles 均已成功產生 ARM ELF；Pico 2 仍待購買後實機測試。
- Flash runtime profile、Save Configuration 與自動偵測仍屬 persistent configuration 階段。

## 8. 尚未實現或仍待確認

- Production VID/PID 與正式產品名稱。
- RP2040-Zero GP0／GP1 的現場接線確認。
- Bootloader manifest、chunk、flow control、斷線 resume 與降版 override opcode。
- Application 應執行哪些內部自我檢查才可升格 known-good。
- 簽章、加密及金鑰管理。
- Factory reset 與個別設定欄位的清除規則。
- 真正 Bootloader、A/B linker scripts、host test sender 及 HIL firmware。
