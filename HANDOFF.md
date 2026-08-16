# Handoff

## Working-tree state

The current branch is `feature/phaseII/architecture-refactor` at committed HEAD
`96a8c1b`. Stage 5 is implemented but uncommitted. The working tree also contains
the deliberate deletion of tracked CMake artifacts under `build_check/` and
`build_check2/`, plus a VS Code Markdown-preview setting.

Stage 5 currently includes:

- runtime board profile, FA/DA response delay, and incomplete-frame timeout;
- a separate five-byte `0xCF` configuration request and seven-byte response;
- explicit Save to a two-slot, sequence-numbered, CRC32-protected Flash journal;
- startup loading with MCU-family profile restrictions;
- immediate `EC 00 00 00 00 0D 0A` response for an expired partial frame;
- host tests and four-board CMake integration.

## Verification performed

Verified on 2026-08-16:

- Host configure/build succeeded.
- CTest passed 6/6: parser, application protocol, protocol service, board
  profile, runtime configuration, and characterization contract.
- Pico, Pico W, Waveshare RP2040-Zero, and Pico 2 all produced `host.elf` with
  Pico SDK 2.1.1 in `elf-only` mode.

No physical hardware test was performed. UF2 generation was not part of this
verification.

## Unfinished work

1. Flash persistence needs HIL coverage: actual save/load, interrupted erase or
   program, corrupt-newest fallback, and endurance expectations.
2. The linker does not yet reserve/assert the final two 4 KiB configuration
   sectors. This must be fixed before firmware growth or the A/B layout can make
   an image overlap them.
3. Pure-noise input and valid-frame-plus-noise still do not implement the
   required one-error-per-batch behavior. Only partial-frame timeout is wired.
4. Factory reset and per-field reset policy are undecided and unimplemented.
5. Requirements call for build-time-configurable response delay and timeout.
   Runtime configuration exists, but public CMake/build options for the timing
   defaults do not.
6. Boot control `0xBC`, bootloader, A/B slots/metadata, rollback, updater, version
   policy, and production USB identity remain unimplemented.
7. USB CDC, GPIO, UART backdoor, board-profile switching, and timeout behavior
   need physical validation on the target boards. Pico 2 remains compile-only.
8. Pico W CYW43 and RP2040-Zero WS2812 status indicators remain disabled.
9. `build_check/` and `build_check2/` were removed, but they are not yet listed
   in `.gitignore`; committing the tracked deletions and adding ignore rules is
   still required if the cleanup is intended to persist.
10. The prior handoff goal—replace fixed temperature responses with a continuous
   sine-wave simulation from -40 to 85—has not been implemented. Production
   responses are still fixed byte arrays in `src/application/app_protocol.c`,
   and characterization vectors still require those fixed bytes. Before coding,
   define the numeric encoding, which temperature commands change, step size,
   rounding, phase reset behavior, and compatibility-vector changes.

## Recommended next steps

1. Review and commit Stage 5 separately from generated-build cleanup/editor
   settings if clean history is desired.
2. Add the linker reservation/assertion and host-test any address calculations.
3. Run Stage 5 HIL tests before treating Flash persistence as release-ready.
4. Resolve the temperature protocol questions before changing its fixed vectors.
