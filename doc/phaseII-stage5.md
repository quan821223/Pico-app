# 第五階段：執行期設定與 Flash 持久化

## 交付範圍

本階段把原本寫死的 FA／DA 回應延遲與不完整封包逾時抽離成
`runtime_config`，並加入板型設定。設定命令屬於獨立的 `0xCF`
控制命名空間，不會進入既有 FA／DA application dispatcher。

已實作：

- 回應延遲預設 50 ms，只套用於 FA／DA 回應。
- 不完整封包逾時預設 100 ms；逾時回覆
  `EC 00 00 00 00 0D 0A`，且不套用 50 ms 延遲。
- 板型、回應延遲及逾時可讀取、修改。
- 修改先存在 RAM；只有明確 Save 指令才寫入 Flash。
- 雙 Flash sector journal、sequence 與 CRC32，可在最新紀錄損毀時
  回復上一筆有效紀錄。
- RP2040 韌體只接受 Pico、Pico W、Waveshare RP2040-Zero；RP2350
  韌體只接受 Pico 2，避免跨 MCU 套用不相容 profile。

尚未實作：純雜訊批次的單次錯誤回覆、Bootloader/A/B metadata、USB
序號與韌體版本保存。這些不能誤認為已由本階段完成。

## 0xCF 設定協定

所有 request 固定五 bytes：

`CF OP KEY VALUE_H VALUE_L`

- `OP=52`：讀取；request value 應填 `0000`。
- `OP=57`：寫入。
- `KEY=01`：board profile，值為 0 Pico、1 Pico W、2 Waveshare
  RP2040-Zero、3 Pico 2。
- `KEY=02`：FA／DA response delay，單位 ms，範圍 0..5000。
- `KEY=03`：incomplete-frame timeout，單位 ms，範圍 1..5000。
- `KEY=7E`：Save configuration；寫入值必須為 `A55A`。

response 固定七 bytes：

`CF STATUS KEY VALUE_H VALUE_L 0D 0A`

狀態碼為 `00` 成功、`01` 未知 key、`02` 值或 operation 不合法、`03`
儲存失敗。讀取 request 的 value 欄位目前不驗證。控制回應立即排程，
不使用 FA／DA 的模擬延遲。

延遲及逾時在寫入成功後立即作用。板型設定可保存，但 GPIO/UART
只在重新開機時重新初始化，因此板型於下一次啟動生效。

## Flash 紀錄與斷電安全

目前使用 Flash 最後兩個 4 KiB erase sectors，一個 sector 對應一個
slot。24-byte record 的格式如下：

| Offset | Size | 欄位 |
|---:|---:|---|
| 0 | 4 | magic `PCFG` |
| 4 | 2 | schema version（目前 1） |
| 6 | 2 | record size（24） |
| 8 | 4 | sequence |
| 12 | 1 | board ID |
| 13 | 1 | reserved |
| 14 | 2 | response delay ms |
| 16 | 2 | frame timeout ms |
| 18 | 2 | reserved |
| 20 | 4 | 前 20 bytes 的 CRC32/IEEE |

Save 永遠寫到另一個 slot：先 erase、再 program、最後讀回核對。只有
完整寫入成功後才把新 slot 視為目前狀態。因此更新途中斷電時，開機
會忽略 CRC 或 header 不合法的新紀錄並使用上一筆。sequence 的比較
支援 32-bit rollover。

CRC32 只能偵測意外損壞，不能證明韌體或設定來自可信來源；簽章與
加密仍是待決安全項目。

## Flash 壽命與配置界線

每次 Save 會擦除一個 4 KiB sector，所以一般設定變更不自動保存。
上位機應完成多項修改後只送一次 Save，避免不必要的擦寫。最終
Bootloader/A/B linker layout 必須明確排除此二 sector；本階段的配置
不可直接當成最終 A/B 分割表。

## 驗證結果

- 主機端 CTest：6/6 通過。
- 測試涵蓋 CRC 標準向量、最新紀錄選擇、損毀回復、寫入失敗不前進、
  設定命令、MCU 板型限制、控制路由與動態延遲。
- Pico、Pico W、Waveshare RP2040-Zero、Pico 2 均完成 ELF 編譯。
- Flash 實機斷電測試尚待 HIL 階段執行。
- 目前 linker script 尚未硬性排除最後兩個設定 sectors；映像仍小，不會
  重疊，但在韌體成長或導入 A/B 前必須加入 linker reservation/assertion。
