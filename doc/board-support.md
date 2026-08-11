# Board support and pin plan

## Binary compatibility

- Pico, Pico W, and RP2040-Zero use RP2040 and share RP2040 application logic.
- Pico 2 uses RP2350 and requires a separately linked binary.
- Source modules, protocol contracts, and board-profile structures are shared.

## Build interface (implemented)

```powershell
Builder.bat pico
Builder.bat pico_w
Builder.bat waveshare_rp2040_zero
Builder.bat pico2
```

Pico 2 is build-only until hardware is available. `Builder.bat` delegates to
`tools/build_firmware.ps1`; CMake rejects SDK versions other than 2.1.1 and uses
an isolated `build/<board>/` cache for each profile.

## Logical pin table

| Logical function | Pico | Pico W | RP2040-Zero | Pico 2 | Status |
|---|---:|---:|---:|---:|---|
| Chamber output 0 | GP2 | GP2 | GP2 | GP2 | Existing/confirmed |
| Chamber output 1 | GP3 | GP3 | GP3 | GP3 | Existing/confirmed |
| Chamber address 0 | GP18 | GP18 | GP18 | GP18 | Confirm wiring |
| Chamber address 1 | GP19 | GP19 | GP19 | GP19 | Confirm wiring |
| Touch output 0 | GP20 | GP20 | GP20 | GP20 | Confirm wiring |
| Touch output 1 | GP21 | GP21 | GP21 | GP21 | Confirm wiring |
| UART0 TX | GP16 | GP16 | GP0 | GP16 | Profile implemented |
| UART0 RX | GP17 | GP17 | GP1 | GP17 | Profile implemented |
| Status LED | GP25 | CYW43 disabled | WS2812 GP16 disabled | GP25 | Capability modeled |

GP0/GP1 are configured for RP2040-Zero because UART0 supports those functions and
the Waveshare header exposes them. Deployed wiring still requires physical
confirmation before release.

## LED differences

- Pico and Pico 2 use a conventional LED on GP25.
- Pico W LED is controlled through CYW43. It is disabled in Phase II until
  wireless initialization is otherwise required.
- Waveshare RP2040-Zero has a WS2812 RGB LED on GP16. UART TX therefore moves to
  GP0. The WS2812 capability is modeled but its waveform driver remains disabled.

## Runtime selection

- Non-invasive detection may suggest a profile only when evidence is reliable.
- There is no guaranteed universal hardware board ID for Pico versus Pico W.
- Deterministic selection uses a build option or a versioned Flash profile.
- Profile changes take effect after restart; Pico is the safe default.

## Sources

- Raspberry Pi Pico-series documentation: https://www.raspberrypi.com/documentation/microcontrollers/raspberry-pi-pico.html
- Raspberry Pi Pico SDK 2.1.1 board headers.
- Waveshare RP2040-Zero: https://www.waveshare.com/wiki/RP2040-Zero
