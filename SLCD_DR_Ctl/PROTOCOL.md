# SLCD_DR_Ctl UART protocol

UART settings:
- Baud: **250000**
- Data bits: 8
- Parity: none
- Stop bits: 1

## Frame format

Each frame is binary:

```text
SOF1 SOF2 CMD LEN_L LEN_H PAYLOAD... CRC
```

- `SOF1` = `0xA5`
- `SOF2` = `0x5A`
- `CMD`  = command ID
- `LEN_L`, `LEN_H` = payload length (little endian)
- `CRC` = `crc8()` table CRC of `CMD`, `LEN_L`, `LEN_H`, and all payload bytes (initial value `0x00`, `tablCrc8`; check vector `123456789` => `0xA1`)

## Commands (PC -> Arduino)

### `0x01` SET_ALL
Replace payload from index 0. Length can be `0..520`.

Payload:
```text
B0 B1 B2 ... BN
```
Each byte is interpreted as a bit value:
- `0x00` => 0
- any non-zero => 1

### `0x02` SET_RANGE
Update payload range at arbitrary offset.

Payload:
```text
OFF_L OFF_H CNT_L CNT_H DATA0 DATA1 ... DATA(CNT-1)
```
- `offset` = `OFF_L | OFF_H<<8`
- `count` = `CNT_L | CNT_H<<8`
- total payload size must be exactly `4 + count`

### `0x03` SET_PACKED_RANGE
Update all or part of the payload with packed bits. This preserves the `0x01` and `0x02` commands while reducing transfer size by up to 8x.

Payload:
```text
OFF_L OFF_H BIT_CNT_L BIT_CNT_H PACKED0 PACKED1 ... PACKED(ceil(BIT_CNT/8)-1)
```
- `offset` = `OFF_L | OFF_H<<8`, expressed in payload bits.
- `bit_count` = `BIT_CNT_L | BIT_CNT_H<<8`.
- total payload size must be exactly `4 + ceil(bit_count / 8)`.
- bit order is little-endian inside each packed byte: the first payload bit is bit 0 of `PACKED0`, the second is bit 1, and so on.
- use `offset = 0` and `bit_count = 520` to set the whole pattern.

### `0x10` GET_INFO
No payload.

### `0x11` GET_RANGE
Payload:
```text
OFF_L OFF_H CNT_L CNT_H
```

## Responses (Arduino -> PC)

### `0x80` ACK
Payload: `CMD_ECHO`

### `0x81` NACK
Payload: `ERR`

Error codes:
- `0x01` invalid SET_RANGE payload (too short)
- `0x02` invalid SET_RANGE length mismatch
- `0x03` invalid GET_RANGE request length
- `0x04` requested range count too large
- `0x07` invalid SET_PACKED_RANGE payload (too short)
- `0x08` invalid SET_PACKED_RANGE length mismatch
- `0x05` payload too large for parser
- `0x06` CRC mismatch
- `0x7F` unknown command

### `0x90` INFO
Payload:
```text
SIZE_L SIZE_H
```
Current payload capacity (520).

### `0x91` RANGE_DATA
Payload: bytes with requested payload content.

## Typical update flow

1. Send `GET_INFO` to get payload size.
2. Build byte payload in host app (`0`/`1` bytes).
3. Prefer `SET_PACKED_RANGE` (`0x03`) to set the full frame or partial ranges with packed bytes. Legacy `SET_ALL` and `SET_RANGE` remain available for unpacked `0`/`1` byte payloads.
4. Wait for `ACK`.


### `0x20` SET_MODE
Enter or leave the high-level interactive mode without changing the legacy raw-payload commands.

Payload:
```text
MODE
```
- `0x00` = raw payload mode.
- `0x01` = interactive mode.

### `0x21` INTERACTIVE_SET
Compressed high-level control for the same segment groups used by `orig_display.cpp` on `A2560_BoardR2`. The command is valid after `SET_MODE 0x01`; each field is packed as one byte of field ID plus a signed 32-bit little-endian value. Multiple fields can be sent in a single frame.

Payload:
```text
FIELD0 VALUE0_L VALUE0_1 VALUE0_2 VALUE0_H [FIELD1 VALUE1_L ...]
```

Field IDs:
- `0x01` speedometer value (`0..999`)
- `0x02` RPM value
- `0x03` MFA type (`0` duration, `1` km, `2` l/100, `3` km/h, `4` oil, `5` air, `6` none)
- `0x04` MFA displayed number (signed)
- `0x05` MFA clock as `(hours << 8) | minutes`
- `0x06` fuel litres (`0..99`)
- `0x07` coolant bar count (`0..20`)
- `0x08` mileage (`0..999999`)
- `0x09` clock as `(hours << 8) | minutes`
- `0x0A` clock/MFA dot (`0` off, non-zero on)
- `0x0B` MFA float dot (`0` off, non-zero on)
- `0x0C` refuel sign (`0` off, non-zero on)
- `0x0D` MFA block placeholder; accepted for protocol compatibility, but SLCD_DR_Ctl has no external MFA block pins
- `0x0E` backlight placeholder; accepted for protocol compatibility, but SLCD_DR_Ctl has no external backlight pin
- `0x0F` max RPM scale for the RPM bar (defaults to `7000`)

Additional `NACK` error codes:
- `0x09` invalid SET_MODE payload length
- `0x0A` invalid INTERACTIVE_SET payload length
- `0x0B` unsupported interactive field or interactive mode is not enabled
