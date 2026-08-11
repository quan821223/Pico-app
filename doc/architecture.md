# Proposed architecture

## Goals

- Keep protocol behavior independent of USB, UART, GPIO, clocks, and Flash.
- Make deterministic logic executable as native host tests.
- Isolate board differences behind a stable hardware abstraction.
- Share application logic between RP2040 and RP2350 without treating their
  binaries as interchangeable.

## Dependency direction

```text
app/main
  -> application services
       -> protocol parser -> command dispatcher -> response encoder
       -> board service
       -> configuration service
       -> boot-control service
  -> interfaces (USB CDC, UART debug)
  -> platform adapters (RP2040/RP2350, Flash, GPIO, clock, watchdog)
```

Core protocol and state-machine modules may depend on interfaces and plain data
types, but never on Pico SDK headers. Pico SDK code implements those interfaces
at the outer edge.

## Planned modules

- `protocol`: byte-stream framing, timeout, validation, command and response DTOs.
- `application`: FA/DA behavior and IO-independent command rules.
- `transport`: USB CDC and optional UART mirror adapters.
- `board`: selected profile and logical-signal-to-pin mapping.
- `config`: versioned, CRC-protected persistent records with two-copy recovery.
- `boot_control`: request validation and reboot handoff metadata.
- `bootloader`: receiver, CRC, version policy, slot manager, and launcher.
- `platform`: Pico SDK clock, Flash, watchdog, unique ID, GPIO, and USB adapters.

## Implemented through stage 3

- `src/protocol`: pure C stream parser.
- `src/application/app_protocol`: pure C decoder, response encoder, and typed effects.
- `src/application/protocol_service`: ports-based orchestration between parser,
  inputs, effects, and response delivery.
- `src/platform/pico/pico_app_io`: Pico GPIO/timer implementation for Chamber,
  Touch, and the RAM-only backdoor.
- `src/main.c`: composition root and remaining USB/UART transport concerns.
- `src/board/board_profile`: pure C board identities, pins, capabilities, MCU,
  and Flash size; selected by a validated CMake mapping.

The former `tud_cdc_descript.*` and `ALL.h` mixed-responsibility files were
removed after their responsibilities moved to these modules.

## Configuration precedence

The implementation must retain an override buffer for later field needs:

1. Explicit build-time locked profile, when supplied for diagnostics.
2. Valid saved Flash profile.
3. Build-time default profile.
4. Safe Raspberry Pi Pico fallback.

Exact flags and factory-reset semantics remain subject to implementation review.
No application source edit should be required to select a board.

## Compatibility gate

Production refactoring starts after review of the characterization contract.
Each migrated command must pass the same vectors before old code is removed.
Approved differences receive a change-control entry and a new vector.
