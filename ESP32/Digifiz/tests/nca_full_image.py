#!/usr/bin/env python3
"""Compose an initially bootable 32 KiB NCA bootloader/application image."""

from __future__ import annotations

import argparse
import struct
import zlib
from pathlib import Path


FLASH_BASE = 0x08000000
FLASH_SIZE = 32 * 1024
BOOTLOADER_SIZE = 4 * 1024
APPLICATION_OFFSET = BOOTLOADER_SIZE
METADATA_OFFSET = 31 * 1024
MAX_APPLICATION_SIZE = METADATA_OFFSET - APPLICATION_OFFSET
SRAM_START = 0x20000000
SRAM_END = 0x20002000
IMAGE_MAGIC = 0x4E434131
APPLICATION_ID_MAGIC = 0x4941434E
APPLICATION_ID_VERSION = 1


def validate_vectors(image: bytes, base_address: int, region_size: int,
                     description: str) -> None:
    if len(image) < 8 or len(image) > region_size:
        raise ValueError(
            f"{description} size must be 8..{region_size} bytes, got {len(image)}"
        )
    initial_sp, reset_vector = struct.unpack_from("<II", image)
    reset_address = reset_vector & ~1
    if not SRAM_START <= initial_sp <= SRAM_END:
        raise ValueError(
            f"{description} has invalid initial stack pointer 0x{initial_sp:08X}"
        )
    if not reset_vector & 1 or not base_address <= reset_address < base_address + len(image):
        raise ValueError(
            f"{description} reset vector 0x{reset_vector:08X} is outside its image"
        )


def metadata_for(application: bytes) -> bytes:
    checksum = zlib.crc32(application) & 0xFFFFFFFF
    return struct.pack(
        "<IIII", len(application), checksum, checksum ^ 0xFFFFFFFF, IMAGE_MAGIC
    )


def validate_bootable_application(application: bytes) -> None:
    validate_vectors(
        application, FLASH_BASE + APPLICATION_OFFSET,
        MAX_APPLICATION_SIZE, "bootable application",
    )
    identity = struct.pack(
        "<III", APPLICATION_ID_MAGIC, FLASH_BASE + APPLICATION_OFFSET,
        APPLICATION_ID_VERSION,
    )
    if application.count(identity) != 1:
        raise ValueError(
            "application has no unique NCA bootable linked-origin identity; "
            "a standalone image cannot be used"
        )


def compose_full_image(bootloader: bytes, application: bytes) -> bytes:
    validate_vectors(bootloader, FLASH_BASE, BOOTLOADER_SIZE, "bootloader")
    validate_bootable_application(application)
    image = bytearray(b"\xFF" * FLASH_SIZE)
    image[:len(bootloader)] = bootloader
    image[APPLICATION_OFFSET:APPLICATION_OFFSET + len(application)] = application
    metadata = metadata_for(application)
    image[METADATA_OFFSET:METADATA_OFFSET + len(metadata)] = metadata
    return bytes(image)


def ihex_record(address: int, record_type: int, data: bytes) -> str:
    body = bytes((len(data), address >> 8, address & 0xFF, record_type)) + data
    checksum = (-sum(body)) & 0xFF
    return ":" + (body + bytes((checksum,))).hex().upper()


def write_ihex(path: Path, regions: list[tuple[int, bytes]]) -> None:
    lines: list[str] = []
    current_upper = None
    for address, data in regions:
        position = 0
        while position < len(data):
            absolute = address + position
            upper = absolute >> 16
            if upper != current_upper:
                lines.append(ihex_record(0, 4, struct.pack(">H", upper)))
                current_upper = upper
            chunk = data[position:position + min(16, 0x10000 - (absolute & 0xFFFF))]
            lines.append(ihex_record(absolute & 0xFFFF, 0, chunk))
            position += len(chunk)
    lines.append(ihex_record(0, 1, b""))
    path.write_text("\n".join(lines) + "\n", encoding="ascii")


def build(bootloader_path: Path, application_path: Path,
          binary_path: Path, hex_path: Path) -> None:
    bootloader = bootloader_path.read_bytes()
    application = application_path.read_bytes()
    full_image = compose_full_image(bootloader, application)
    binary_path.write_bytes(full_image)
    write_ihex(
        hex_path,
        [
            (FLASH_BASE, bootloader),
            (FLASH_BASE + APPLICATION_OFFSET, application),
            (FLASH_BASE + METADATA_OFFSET, metadata_for(application)),
        ],
    )
    checksum = zlib.crc32(application) & 0xFFFFFFFF
    print(
        f"Created {binary_path.name} and {hex_path.name}: "
        f"application={len(application)} bytes CRC-32=0x{checksum:08X}"
    )


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("bootloader", type=Path)
    parser.add_argument("application", type=Path)
    parser.add_argument("output_binary", type=Path)
    parser.add_argument("output_hex", type=Path)
    args = parser.parse_args()
    build(args.bootloader, args.application, args.output_binary, args.output_hex)


if __name__ == "__main__":
    main()
