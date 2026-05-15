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

