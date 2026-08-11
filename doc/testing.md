# Test strategy

## Host unit and contract tests

Host tests cover stream parsing, resynchronization, timeout, FA/DA dispatch,
byte-exact responses, invalid batches, board profiles, persistent records, CRC32,
version policy, and A/B transitions. Stage 2 made the stream parser native;
Stage 3 also made the production command logic and protocol service native. The
JSON contract invokes a compiled production runner for byte-exact validation,
while mock ports verify inputs, effects, and response delivery.

## Two-board HIL

Two modes are required:

1. UART fixture mode uses GPIO serial pins for UART backdoor tests.
2. USB-host fixture mode runs TinyUSB host on a controller Pico and connects to
   the device under test. This exercises enumeration, CDC framing, reconnect,
   bootloader identity, and update flow.

UART pins are not electrically or behaviorally equivalent to USB CDC. Reports
must state which transport was exercised.

## Raspberry Pi and SWD

Research and document Raspberry Pi GPIO/OpenOCD-style SWD and a Pico running
official Debug Probe firmware. Pico and Pico W expose SWD. Waveshare RP2040-Zero
does not expose standard SWD pads, so it uses USB/UART black-box acceptance unless
hardware access is later demonstrated.

## Planned layout and CI

```text
tests/characterization/
tests/unit/
tools/hil/
tools/update_host_reference/
```

The production host updater is out of scope. A minimal reference/test sender is
still required to verify the device protocol and does not replace the user's UI.
GitHub Actions will run host tests and compile profiles with Pico SDK 2.1.1.
Hardware tests run on a self-hosted runner or documented local setup. Pico 2 is
compile-only until hardware is available.

- Existing behavior must match reviewed characterization vectors.
- Approved differences need a change-control entry and tests.
- Hardware claims remain unverified until the named board and transport pass HIL.
