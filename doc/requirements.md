# Confirmed requirements

## Scope

- Refactor the complete firmware project from `develop@8d3920f`.
- Keep existing FA/DA request and response bytes compatible unless a change is
  recorded in `change-control.md`.
- Preserve the deployed USB CDC application behavior and the UART chamber
  backdoor.
- Continue supporting Windows batch-file builds.
- Make builds reproducible without relying on an undocumented personal
  `PICO_SDK_PATH`; Pico SDK 2.1.1 remains the pinned SDK.
- Document the design, trade-offs, implemented features, and unimplemented
  features under `doc/`.

## Boards

- Required now: Raspberry Pi Pico, Raspberry Pi Pico W, and Waveshare
  RP2040-Zero.
- Future-ready: Raspberry Pi Pico 2. It shares the architecture and protocol,
  but requires a separate RP2350 binary.
- Support a simple build-time board selection and a persistent runtime board
  profile. Board changes take effect after restart.
- The safe default profile is Raspberry Pi Pico.

## Application protocol

- Preserve the fixed five-byte FA/DA command format and current resynchronizing
  behavior.
- Default simulated FA/DA response delay remains 50 ms.
- Response delay is configurable at build time and runtime and can be saved.
- IO actions, error responses, UART acknowledgements, boot control, and
  bootloader transfer responses do not use the simulated 50 ms delay.
- Incomplete-frame timeout defaults to 100 ms and is build-time configurable,
  runtime configurable, and persistable.
- Multiple complete frames in one USB read are parsed independently.
- One valid frame followed by noise produces the valid response and one error.
- One batch of noise produces one error response.
- New control/configuration commands must not use FA or DA and must be documented.

Current Stage 5 status: runtime changes and explicit Flash persistence are
implemented for board profile, response delay, and incomplete-frame timeout.
The timing defaults are compile-time macros, but user-facing CMake/build options
for overriding delay and timeout are not implemented yet. Pure-noise and
valid-frame-plus-noise batch error behavior also remains unimplemented.

## Planned bootloader

- Application boot-entry request uses the reserved `0xBC` control header.
- Candidate request: `BC 57 01 00 01`; application replies `C3 0D 0A`, flushes
  USB, then restarts without waiting for another host acknowledgement.
- Bootloader USB product name: `Pico Bootloader`; application name: `Pico App`.
- Application and bootloader use different PIDs. Production VID/PID assignment
  remains open; test identifiers must be clearly marked non-production.
- USB serial number is derived from the MCU unique ID.
- Bootloader waits 60 seconds for an update and otherwise boots the valid
  application.
- Firmware transfer uses CRC32 for corruption detection.
- Downgrade is rejected normally and permitted through a documented special
  control command.
- No external authorization is required to enter bootloader mode.
- Encryption and signing are currently unimplemented/open security items.
- Native BOOTSEL/UF2 is the final manual recovery path, not the normal updater.
- A/B recovery is mandatory: keep a known-good image and try a candidate at most
  three times before rollback.

## Testing and delivery

- Add host unit tests for parser, dispatch, response bytes, invalid data, CRC,
  version policy, and A/B state transitions.
- Add characterization tests before changing production behavior.
- Add RP2040 hardware-in-the-loop tests for USB CDC, GPIO, UART, restart,
  interrupted update, and rollback.
- Add GitHub Actions host tests and per-board compilation checks.
- Pico 2 initially requires compilation and documentation only; physical tests
  wait for hardware.
- HIL assets belong under `tools/hil/`.
- Existing defects that would change externally visible output require an entry
  in `change-control.md` before correction.

## Open requirements

- Exact production USB VID/PID and final product branding.
- Final allocation of GP0/GP1 for RP2040-Zero UART after wiring verification.
- Exact bootloader transfer framing and downgrade-control command.
- Concrete application self-tests used to confirm a candidate image.
- Whether an external Flash hardware revision is permissible if internal A/B
  sizing ever fails.
- Which persistent fields should be resettable individually and the factory
  reset behavior.
