#!/usr/bin/env python3
"""Example one-way NCA interactive telemetry orchestrator."""

from __future__ import annotations

import argparse
import math
import struct
import time
from collections import defaultdict

import serial


BAUD_RATE = 250_000
BROADCAST = 0x5A
PROTOCOL_VERSION = 1

CMD_PUBLISH = 0x10
CMD_SET_MODE = 0x20
CMD_SET_CONFIG = 0x21

MODE_RAW = 0
MODE_INTERACTIVE = 1

TOPIC_RPM = 0x01
TOPIC_SPEED = 0x02
TOPIC_OIL_TEMPERATURE = 0x03
TOPIC_OUTDOOR_TEMPERATURE = 0x04
TOPIC_COOLANT_TEMPERATURE = 0x05
TOPIC_FUEL_LEVEL = 0x06
TOPIC_INDICATORS = 0x07
TOPIC_GENERIC_BASE = 0x20

CONFIG_MINIMUM_F32 = 0x01
CONFIG_MAXIMUM_F32 = 0x02

DEVICE_COOLANT = 1
DEVICE_SPEEDOMETER_BLOCK = 2
DEVICE_SPEEDOMETER = 3
DEVICE_INDICATORS = 4
DEVICE_RPM = 5
DEVICE_OUTDOOR = 6
DEVICE_OIL = 7
DEVICE_FUEL = 8


def make_frame(destination: int, command: int, payload: bytes = b"") -> bytes:
    """Build the common A5/address/command/length/payload/XOR envelope."""
    if not 0 <= destination <= 0xFF:
        raise ValueError("destination must fit in one byte")
    if len(payload) > 0xFFFF:
        raise ValueError("payload is too large")
    length = struct.pack("<H", len(payload))
    checksum = command ^ length[0] ^ length[1]
    for byte in payload:
        checksum ^= byte
    return bytes((0xA5, destination, command)) + length + payload + bytes((checksum,))


def signed_deci(value: float) -> bytes:
    scaled = round(value * 10.0)
    if not -32768 <= scaled <= 32767:
        raise ValueError("signed deci-unit value is outside int16_t")
    return struct.pack("<h", scaled)


class NcaOrchestrator:
    def __init__(self, port: serial.Serial, parcel_delay: float) -> None:
        self.port = port
        self.parcel_delay = parcel_delay
        self.sequence: dict[int, int] = defaultdict(int)
        self.last_write = 0.0

    def send(self, destination: int, command: int, payload: bytes = b"") -> None:
        remaining = self.parcel_delay - (time.monotonic() - self.last_write)
        if remaining > 0.0:
            time.sleep(remaining)
        self.port.write(make_frame(destination, command, payload))
        self.port.flush()
        self.last_write = time.monotonic()

    def publish(self, topic: int, data: bytes) -> None:
        sequence = self.sequence[topic]
        self.sequence[topic] = (sequence + 1) & 0xFFFF
        payload = struct.pack("<BBH", PROTOCOL_VERSION, topic, sequence) + data
        self.send(BROADCAST, CMD_PUBLISH, payload)

    def set_mode(self, mode: int, destination: int = BROADCAST) -> None:
        self.send(destination, CMD_SET_MODE, bytes((mode,)))

    def set_range(self, destination: int, minimum: float, maximum: float) -> None:
        # Topic zero means "the topic assigned to this device at build time".
        payload = bytes((PROTOCOL_VERSION, 0))
        payload += bytes((CONFIG_MINIMUM_F32, 4)) + struct.pack("<f", minimum)
        payload += bytes((CONFIG_MAXIMUM_F32, 4)) + struct.pack("<f", maximum)
        self.send(destination, CMD_SET_CONFIG, payload)


def configure_cluster(orchestrator: NcaOrchestrator) -> None:
    orchestrator.set_mode(MODE_RAW)
    ranges = (
        (DEVICE_COOLANT, -40.0, 130.0),
        (DEVICE_RPM, 0.0, 8000.0),
        (DEVICE_OUTDOOR, -50.0, 60.0),
        (DEVICE_OIL, -40.0, 160.0),
        (DEVICE_FUEL, 0.0, 100.0),
    )
    for address, minimum, maximum in ranges:
        orchestrator.set_range(address, minimum, maximum)
    orchestrator.set_mode(MODE_INTERACTIVE)


def run_demo(orchestrator: NcaOrchestrator, duration: float) -> None:
    """Publish example changing values at the nominal protocol rates."""
    start = time.monotonic()
    due = {
        "rpm": start,
        "speed": start,
        "slow": start,
        "indicators": start,
        "generic": start,
    }
    generic_index = 0

    while duration <= 0.0 or time.monotonic() - start < duration:
        now = time.monotonic()
        elapsed = now - start

        if now >= due["rpm"]:
            rpm = round(900.0 + 6500.0 * (0.5 + 0.5 * math.sin(elapsed * 0.7)))
            orchestrator.publish(TOPIC_RPM, struct.pack("<H", rpm))
            due["rpm"] = max(due["rpm"] + 1.0 / 30.0, now)

        if now >= due["speed"]:
            speed = 130.0 * (0.5 + 0.5 * math.sin(elapsed * 0.22))
            orchestrator.publish(TOPIC_SPEED, struct.pack("<H", round(speed * 10.0)))
            due["speed"] = max(due["speed"] + 0.2, now)

        if now >= due["indicators"]:
            phase = int(elapsed) % 8
            first_word = (1 << phase) | (1 << (16 + (phase % 4)))
            second_word = 1 << ((phase + 3) % 16)
            orchestrator.publish(TOPIC_INDICATORS,
                                 struct.pack("<II", first_word, second_word))
            due["indicators"] = max(due["indicators"] + 1.0, now)

        if now >= due["slow"]:
            orchestrator.publish(TOPIC_OIL_TEMPERATURE,
                                 signed_deci(95.0 + 8.0 * math.sin(elapsed / 30.0)))
            orchestrator.publish(TOPIC_OUTDOOR_TEMPERATURE,
                                 signed_deci(18.5 + 4.0 * math.sin(elapsed / 50.0)))
            orchestrator.publish(TOPIC_COOLANT_TEMPERATURE,
                                 signed_deci(88.0 + 5.0 * math.sin(elapsed / 25.0)))
            orchestrator.publish(TOPIC_FUEL_LEVEL,
                                 signed_deci(max(0.0, 78.0 - elapsed / 120.0)))
            due["slow"] = max(due["slow"] + 10.0, now)

        if now >= due["generic"]:
            value = generic_index * 10.0 + 2.5 * math.sin(elapsed)
            orchestrator.publish(TOPIC_GENERIC_BASE + generic_index,
                                 signed_deci(value))
            generic_index = (generic_index + 1) % 10
            # One channel every 0.5 s means each of the ten channels is sent
            # once every 5 s without a ten-frame burst.
            due["generic"] = max(due["generic"] + 0.5, now)

        next_due = min(due.values())
        time.sleep(max(0.0, min(0.002, next_due - time.monotonic())))


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("port", help="UART port, for example COM4 or /dev/ttyUSB0")
    parser.add_argument("--duration", type=float, default=0.0,
                        help="seconds to run; zero runs until Ctrl+C")
    parser.add_argument("--parcel-delay-ms", type=float, default=2.0,
                        help="minimum quiet time between UART frames (default: 2 ms)")
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    if args.duration < 0.0 or args.parcel_delay_ms < 0.0:
        raise SystemExit("duration and parcel delay must not be negative")
    with serial.Serial(args.port, BAUD_RATE, bytesize=8, parity="N", stopbits=1,
                       timeout=0, write_timeout=1) as port:
        orchestrator = NcaOrchestrator(port, args.parcel_delay_ms / 1000.0)
        configure_cluster(orchestrator)
        try:
            run_demo(orchestrator, args.duration)
        except KeyboardInterrupt:
            pass


if __name__ == "__main__":
    main()
