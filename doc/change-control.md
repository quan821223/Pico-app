# Compatibility change control

## Approved changes from baseline

| ID | Change | Reason | Status |
|---|---|---|---|
| CC-001 | Invalid batches return `EC 00 00 00 00 0D 0A` | Deterministic errors | Confirmed, not implemented |
| CC-002 | Partial request expires after configurable 100 ms | Expire fragments | Implemented and host-tested; HIL pending |
| CC-003 | New `0xBC` control namespace | Separate control from FA/DA | Confirmed, not implemented |
| CC-004 | ZERO UART GP16/17 to GP0/1 | GP16 drives WS2812 | Implemented, pending physical wiring check |
| CC-005 | Pico W status LED disabled | Avoid CYW43 just for LED | Implemented |
| CC-006 | 66-byte Touch response uses a 66-byte delayed buffer | Remove 2-byte memory overwrite without changing output | Implemented, host/build verified |
| CC-007 | Unsupported FA information parameter is silent | Baseline returned uninitialized heap bytes; unsafe behavior cannot be preserved | Implemented, pending hardware confirmation |
| CC-008 | Touch GPIO is explicitly initialized output/low | Baseline did not reliably configure output direction | Implemented, pending hardware confirmation |
| CC-009 | Non-GPIO indicators are capability-modeled and no-op | Prevent GP25/GP16 misuse on Pico W and Zero | Implemented; drivers remain open |
| CC-010 | New `0xCF` configuration namespace | Separate runtime configuration from FA/DA | Implemented and host-tested; HIL pending |

## Must not silently change

- Supported FA/DA request-to-response bytes.
- Response framing and device-field substitution.
- New-header resynchronization behavior.
- UART chamber backdoor commands and reset-to-disabled behavior.
- GPIO effects, except approved board-profile pin changes.

## Open decision register

| ID | Decision | Blocking stage |
|---|---|---|
| OD-001 | Production USB VID/PID and final product name | Production release |
| OD-002 | Confirm ZERO GP0/GP1 are free in deployed wiring | Board adapter |
| OD-003 | Final updater manifest/chunk protocol and downgrade opcode | Bootloader |
| OD-004 | Exact application self-tests before confirmation | A/B implementation |
| OD-005 | Factory reset and per-field persistence policy | Configuration |
| OD-006 | Whether hardware can add external Flash | Only if sizing fails |
