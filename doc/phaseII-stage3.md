# Phase II 第三階段：Application Protocol 架構化重構

## 架構成果

舊版 `tud_cdc_descript.c` 同時負責 parser、command dispatch、response table、heap allocation、GPIO、timer、Flash-independent state 與延遲排程。第三階段已移除該混合模組與 `ALL.h`，改為：

```text
src/
  application/
    app_protocol.c/.h       純 C command 與 response/effect 規則
    protocol_service.c/.h   parser 與 application ports 協調層
  protocol/
    protocol_stream_parser.c/.h
  platform/pico/
    pico_app_io.c/.h        Pico SDK GPIO、timer、chamber backdoor
  main.c                    composition root、USB CDC、UART、排程
```

依賴方向為 `main/platform -> protocol_service -> app_protocol/protocol parser`。`application` 與 `protocol` 目錄可以由 PC compiler 建置，不包含 Pico SDK header。

## 資料模型

`app_protocol_handle()` 接收固定 5-byte request、唯讀 inputs，輸出：

- 最大 66 bytes 的固定容量 response；
- 是否有 response；
- 一個具型別的 application effect，例如 Touch 0、Touch 1 或 Chamber state。

核心不直接呼叫 GPIO，也不配置 heap。`protocol_service` 透過 function ports 讀取 chamber status、套用 effect、交付延遲 response。PC tests 以 mock ports 驗證這些互動。

## 記憶體政策

- 所有固定協定資料是 `static const`，儲存在唯讀區。
- request path 已完全移除 `malloc/free`。
- response capacity 由 `APP_PROTOCOL_MAX_RESPONSE_SIZE` 定義為 66。
- `_Static_assert` 確保最大 Touch response 與容量一致。
- composition root 的 delayed response 同樣配置 66 bytes，修復舊版 66 bytes 寫入 64-byte buffer 的越界問題。

## 相容性證據

`protocol_vectors.json` 已由 18 組擴充到 42 組，涵蓋：

- FA identification、版本、亮度、diagnostic、current、touch、ALS/CCT；
- DA status、current、voltage、chamber；
- FA/DA write ACK、Touch/Chamber effects；
- 既有 silent cases 與 default ACK。

CTest 會建置 `protocol_contract_runner`，Python contract test 再逐筆把 JSON request 傳給真正的 `app_protocol_handle()`，比對完整 response bytes，不是比對另一份 test-only 模擬器。

## 有意修正

1. 舊版 Touch response 長度 66，但 delayed buffer 只有 64，會造成越界；新架構容量統一為 66，不改變合法輸出。
2. 舊版每個 response 都 `malloc`，新架構使用固定結果物件，消除 fragmentation 與 allocation failure。
3. 舊版 FA information parameter 大於 6 時會傳送未初始化 heap bytes，屬於不可定義且不可安全保留的行為；新架構對此保持 silent，等待未來統一接入 `EC` invalid response。
4. Touch GPIO 現在由 `pico_app_io_init()` 明確初始化為 output/low；舊程式只呼叫 `gpio_put()`，未建立可靠方向設定。

## 仍在 composition root 的責任

`main.c` 目前仍包含 USB CDC 傳輸、UART mirror/backdoor、LED heartbeat 與單一 delayed response slot。它已不包含 command 規則或板級 Chamber/Touch 邏輯。後續可再拆出 transport services 與 board profile，但不需要重新修改 application protocol core。

