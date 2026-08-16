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

## Module status

- `protocol`: framing, resynchronization, and timeout are implemented; complete
  noise-batch error accounting remains open.
- `application`: FA/DA rules and ports-based orchestration are implemented.
- `transport`: USB CDC and UART concerns remain in `main.c`; a separate transport
  module is not implemented.
- `board`: profiles, pin mapping, capabilities, and build selection are implemented.
- `config`: runtime timing/profile values and two-slot CRC persistence are implemented.
- `boot_control` and `bootloader`: planned, not implemented.
- `platform`: Pico GPIO/status and configuration Flash adapters are implemented;
  watchdog and unique-ID adapters remain planned.

## Implemented through stage 5

- `src/protocol`: pure C stream parser.
- `src/application/app_protocol`: pure C decoder, response encoder, and typed effects.
- `src/application/protocol_service`: ports-based orchestration between parser,
  inputs, effects, and response delivery.
- `src/platform/pico/pico_app_io`: Pico GPIO/timer implementation for Chamber,
  Touch, and the RAM-only backdoor.
- `src/main.c`: composition root and remaining USB/UART transport concerns.
- `src/board/board_profile`: pure C board identities, pins, capabilities, MCU,
  and Flash size; selected by a validated CMake mapping.
- `src/config`: runtime values, `0xCF` configuration service, CRC32, and a
  two-slot journal with newest-valid-record recovery.
- `src/platform/pico/pico_config_storage`: final-two-sector RP2040/RP2350 Flash
  adapter used by the configuration journal.
- `src/main.c`: loads saved configuration, applies runtime timing, routes control
  frames, and emits the immediate incomplete-frame timeout response.

The former `tud_cdc_descript.*` and `ALL.h` mixed-responsibility files were
removed after their responsibilities moved to these modules.

## Configuration precedence

Current precedence is:

1. A valid saved Flash profile compatible with the compiled MCU family.
2. The profile selected by the build.

An explicit locked diagnostic override and factory-reset semantics remain open.
No application source edit is required to select a build profile.

## Compatibility gate

Production refactoring starts after review of the characterization contract.
Each migrated command must pass the same vectors before old code is removed.
Approved differences receive a change-control entry and a new vector.
