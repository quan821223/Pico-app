# Current protocol baseline

This document describes the firmware at commit `8d3920f`. It is a compatibility
baseline, not a claim that every behavior is desirable.

## Transport and framing

- Primary transport: USB CDC through TinyUSB.
- Debug mirror: every received and transmitted USB byte is mirrored on UART0.
- UART0 baseline: 115200 baud, TX GP16, RX GP17.
- Request length: exactly five bytes.
- Request layout: `[header, operation, device, category, parameter]`.
- Headers: FA (`0xFA`) and DA (`0xDA`).
- Operations: read (`0x52`) and write (`0x57`).
- A new FA or DA byte while collecting a partial request discards that partial
  request and begins a new one.
- Existing parser has no incomplete-frame timeout and silently ignores most
  invalid data.
- Responses are scheduled approximately 50 ms after dispatch. Only one global
  delayed response buffer exists, so a new response can overwrite a pending one.

## Common response framing

- Most read responses begin with the request family header and end `0D 0A`.
- Write acknowledgement: `C3 0D 0A`.
- The third response byte commonly encodes response length semantics inherited
  from the deployed protocol; it must not be normalized without approval.
- Proposed invalid-frame response (not present in baseline):
  `EC 00 00 00 00 0D 0A`.

## Representative locked vectors

The machine-readable compatibility vectors live in
`tests/characterization/protocol_vectors.json`. Dynamic responses are expressed
with masks or named effects rather than invented values.

## GPIO effects

- DA write, device `03`, category `01`: pulse GP20 for `parameter * 100 ms`.
- DA write, device `03`, category `02`: pulse GP21 for `parameter * 100 ms`.
- DA write, device `03`, category decimal `10` (`0x0A` in source intent): set
  chamber outputs GP2/GP3 from the parameter.
- DA read category/device mapping includes chamber address value from GP18/GP19.

## UART chamber backdoor

UART accepts raw five-byte commands independently of USB:

- `DA 57 03 FF 00`: disable simulated chamber status.
- `DA 57 03 FF 01`: enable simulated chamber status; initial value becomes 3.
- `DA 57 03 0A 00..03`: set simulated status.
- Supported commands return `C3 0D 0A` on UART.
- State is RAM-only. It is disabled again after reset or power loss.

## Known baseline risks

- Protocol, GPIO, allocation, timing, USB, and UART code are tightly coupled.
- Dynamic allocation is used for every response even though response sizes are
  bounded.
- The delayed-response buffer is 64 bytes while one response is 66 bytes.
- A single global delayed response cannot queue back-to-back commands safely.
- Some enum values and comparisons use ambiguous decimal/hex literals.
- Baseline LED uses GP25 directly, which is not the Pico W LED interface.
- RP2040-Zero onboard WS2812 uses GP16, conflicting with baseline UART TX.

These risks are documented only. Changes affecting output require approval and
must be recorded in `change-control.md`.

