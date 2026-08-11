# Planned USB CDC bootloader

Nothing in this document is implemented in the baseline firmware.

## Normal update flow

```text
host sends BC boot-entry request
  -> application sends C3 0D 0A and flushes USB
  -> application records boot request and restarts
  -> bootloader enumerates as "Pico Bootloader" with a distinct PID
  -> host sends manifest and image chunks over bootloader USB CDC
  -> bootloader writes inactive candidate slot
  -> bootloader validates length, target, version, and CRC32
  -> bootloader marks candidate pending and restarts
  -> application performs self-check and confirms itself
  -> confirmed candidate becomes the new known-good image
```

The bootloader waits 60 seconds for an update session. If none starts, it boots
the valid known-good application. Native ROM BOOTSEL/UF2 is the final manual
recovery route.

## Control namespace

- Reserved application control header: `0xBC`.
- Candidate enter request: `BC 57 01 00 01`.
- Application acknowledgement: `C3 0D 0A`, without simulated response delay.
- Downgrade override and configuration opcodes are not yet allocated. They must
  be reviewed and documented before implementation.

## A/B policy

The product model is "known-good plus candidate", even if physical areas remain
named Slot A and Slot B:

1. Known-good is never overwritten during reception.
2. A new image is written to the other slot and verified.
3. The candidate may boot at most three times.
4. Application self-confirmation promotes the candidate to known-good.
5. The former known-good slot becomes the next writable candidate slot.
6. Three failed boots invalidate the candidate and return to known-good.

Promotion changes metadata; it does not copy the full image. This reduces Flash
wear and the power-failure window while preserving the requested A/B behavior.

## Self-confirmation

Host confirmation is not required. USB cable presence is not required. The
application confirms only after its required initialization and internal tests
succeed. Exact tests remain open; candidates include configuration validation,
main-loop start, protocol initialization, and board-profile validation.

There is no user-facing fixed 10-second confirmation step. Internally, reliable
rollback still needs pending state, an attempt counter, reset reason, and an
application-written confirmation record.

## Integrity and security

- CRC32 detects accidental transfer/storage corruption, not malicious changes.
- Encryption and digital signatures are open and unimplemented.
- Boot entry has no password or challenge-response.
- Normal downgrade is rejected; a documented override may allow it.

## File format comparison

- `.bin`: simple fixed-slot streaming, but needs a manifest.
- `.uf2`: excellent ROM BOOTSEL interoperability, but custom slot/version policy
  still needs validation.
- Custom package: carries manifest, version, target, CRC, and payload, but needs
  host tooling.

The proposed Phase II direction is a versioned manifest followed by raw binary
chunks over CDC. It is not final until protocol review.

## Unimplemented items

- Bootloader executable and linker script.
- CDC framing, flow control, resume policy, and host reference tool.
- A/B metadata journal and power-failure tests.
- Downgrade override opcode.
- Signing, encryption, and key management.
- Production VID/PID allocation.

