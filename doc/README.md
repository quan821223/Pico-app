# Phase II refactoring documentation

This directory is the design and compatibility baseline for the Phase II
refactoring. A statement marked **planned** is not an implemented feature.

## Documents

- [phaseII-design.zh-TW.md](phaseII-design.zh-TW.md): 繁體中文核心設計說明與決策理由。
- [phaseII-stage2.md](phaseII-stage2.md): 第二階段純 C parser 的實作與驗證紀錄。
- [phaseII-stage3.md](phaseII-stage3.md): 第三階段 application protocol 與 IO ports 重構紀錄。
- [phaseII-stage4.md](phaseII-stage4.md): 第四階段 board profiles、腳位能力與建置矩陣。
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

## Status vocabulary

- **Existing**: present in the baseline firmware.
- **Confirmed**: explicitly required, but not necessarily implemented.
- **Proposed**: design requiring implementation and verification.
- **Open**: a decision or fact still requiring evidence or approval.
