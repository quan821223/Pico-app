# Flash sizing and provisional layout

## Evidence and estimation method

The baseline `build/host.bin` is 32,444 bytes. ELF length is not used as Flash
occupancy because it includes symbols and debug sections. Final sizing uses:

1. Linker map loadable regions for bootloader and application.
2. Raw `.bin` length rounded up to the Flash erase-sector boundary.
3. A documented growth reserve.
4. Two equal slots sized for the maximum accepted application.
5. Independently erasable, redundant metadata/configuration sectors.
6. Build-time assertions that fail on overlap or overflow.

## RP2040 2 MiB provisional plan

This is a sizing envelope, not a final linker layout:

| Region | Provisional budget | Purpose |
|---|---:|---|
| Bootloader | 128 KiB | USB CDC, Flash writer, CRC, slot selection |
| Slot A | 896 KiB | Known-good or candidate application |
| Slot B | 896 KiB | Candidate or known-good application |
| Config/metadata/log reserve | 128 KiB | redundant records and logs |

Total: 2,048 KiB. The current 32 KiB application is far below the provisional
896 KiB slot ceiling. Budgets change only after real linker maps exist.

## Persistent data

Planned records include board profile, response delay, incomplete-frame timeout,
USB identity derivation/version, firmware version, slot state, boot attempts,
and update result/error. Chamber backdoor remains RAM-only to preserve behavior.

Records use a version, sequence counter, length, and CRC. Two independently
erasable copies allow the newest valid record to survive power loss while the
other is updated. Runtime changes remain in RAM until an explicit Save
Configuration command.

### Implemented application configuration area

Stage 5 reserves the final two 4 KiB Flash sectors for application
configuration. Each sector is an independent journal slot; only one sector is
erased per save. A 24-byte record is programmed in one 256-byte Flash page and
contains magic, schema, length, sequence, board ID, response delay, frame
timeout, and CRC32. On boot, the newest valid sequence is selected. If a write
is interrupted or corrupt, the preceding slot remains usable.

This reservation is an application-stage allocation, not the final A/B linker
layout. The bootloader linker scripts must exclude both sectors and migrate the
record deliberately if the final metadata region moves. Firmware images must
never program these sectors as part of a normal update.

## Why external Flash is deferred

External Flash is a hardware change: every deployed board would need a chip or
module, power, PCB/wiring, available pins, a driver, and new failure handling.
It is considered only if measured internal images cannot meet mandatory A/B.
Current size evidence does not justify it.

## RP2350/Pico 2

Pico 2 has a different MCU and larger official board Flash configuration. It
gets a separate linker layout and binary. RP2350 native partition facilities
will be evaluated without forcing RP2040 into an incompatible implementation.
