# Phase II refactoring documentation

This directory is the design and compatibility baseline for the Phase II
refactoring. A statement marked **planned** is not an implemented feature.

## Documents

- [phaseII-design.zh-TW.md](phaseII-design.zh-TW.md): 繁體中文核心設計說明與決策理由。
- [phaseII-stage2.md](phaseII-stage2.md): 第二階段純 C parser 的實作與驗證紀錄。
- [phaseII-stage3.md](phaseII-stage3.md): 第三階段 application protocol 與 IO ports 重構紀錄。
- [phaseII-stage4.md](phaseII-stage4.md): 第四階段 board profiles、腳位能力與建置矩陣。
- [phaseII-stage5.md](phaseII-stage5.md): 第五階段執行期設定、控制協定與 Flash 雙槽保存。
- [windows-build.zh-TW.md](windows-build.zh-TW.md): Windows 建置、picotool 2.1.1、UF2 產生與常見錯誤的繁體中文操作手冊。
- [requirements.md](requirements.md): confirmed scope, requirements, and open decisions.
- [current-protocol.md](current-protocol.md): behavior observed at the refactoring baseline.
- [architecture.md](architecture.md): proposed module boundaries and dependency rules.
- [board-support.md](board-support.md): board profiles, pin assignments, and build selection.
- [bootloader.md](bootloader.md): planned USB CDC updater and A/B recovery design.
- [flash-layout.md](flash-layout.md): sizing method and provisional RP2040 layout.
- [testing.md](testing.md): host, HIL, SWD, and CI test strategy.
- [change-control.md](change-control.md): compatibility changes and decisions requiring approval.

## Baseline

- Git commit: `8d3920fcb38ba616baee897c2b64b841a295dd85`
- Source branch: `develop` (identical to `main` at the baseline)
- Refactoring branch: `feature/phaseII/architecture-refactor`
- Pico SDK: `2.1.1`
- Existing application binary observed locally: `32,444` bytes (`build/host.bin`)

## Current implementation status

- Stages 2-4 provide the native parser, application protocol/service split,
  Pico IO adapter, board profiles, and four-board build matrix.
- Stage 5 is implemented in the current working tree: runtime board/timing
  configuration, `0xCF` control routing, incomplete-frame timeout response, and
  a two-slot CRC32 Flash journal.
- On 2026-08-16, all 6 host tests passed and Pico, Pico W,
  Waveshare RP2040-Zero, and Pico 2 produced ELF files in `elf-only` mode.
- Flash power-loss behavior, USB/GPIO/UART hardware behavior, and Pico 2 hardware
  behavior have not been physically verified.
- See `../HANDOFF.md` for the exact uncommitted state and remaining work.

## Status vocabulary

- **Existing**: present in the baseline firmware.
- **Confirmed**: explicitly required, but not necessarily implemented.
- **Proposed**: design requiring implementation and verification.
- **Open**: a decision or fact still requiring evidence or approval.
