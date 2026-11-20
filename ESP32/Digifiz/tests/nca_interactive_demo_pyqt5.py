#!/usr/bin/env python3
"""PyQt5 animated demo for the five-device NCA instrument cluster."""

from __future__ import annotations

import struct
import sys
import time
from collections import defaultdict

import serial
import serial.tools.list_ports
from PyQt5 import QtCore, QtWidgets

from nca_interactive_demo import (
    BAUD_RATE,
    BROADCAST,
    CMD_PUBLISH,
    CMD_SET_CONFIG,
    CMD_SET_MODE,
    CONFIG_MAXIMUM_F32,
    CONFIG_MINIMUM_F32,
    DEVICE_COOLANT,
    DEVICE_FUEL,
    DEVICE_RPM,
    DEVICE_SPEEDOMETER_BLOCK,
    MODE_INTERACTIVE,
    MODE_RAW,
    PROTOCOL_VERSION,
    TOPIC_COOLANT_TEMPERATURE,
    TOPIC_FUEL_LEVEL,
    TOPIC_INDICATORS,
    TOPIC_RPM,
    TOPIC_SPEED,
    make_frame,
    signed_deci,
)


DEFAULT_PARCEL_DELAY = 0.002
DEMO_INTERVAL_MS = 100
GAUGE_BAR_COUNT = 9
INDICATOR_COUNT = 24
OIL_TEMPERATURE_HIGH_BIT = 24
OUTDOOR_TEMPERATURE_HIGH_BIT = 25
COOLANT_TEMPERATURE_HIGH_BIT = 26
FUEL_LOW_BIT = 27
GAUGE_WARNING_MASK = sum(
    1 << bit for bit in (
        OIL_TEMPERATURE_HIGH_BIT,
        OUTDOOR_TEMPERATURE_HIGH_BIT,
        COOLANT_TEMPERATURE_HIGH_BIT,
        FUEL_LOW_BIT,
    )
)
RPM_MAXIMUM = 8000
SPEED_MAXIMUM = 300.0
COOLANT_MINIMUM = -40.0
COOLANT_MAXIMUM = 130.0
FUEL_MINIMUM = 0.0
FUEL_MAXIMUM = 100.0


def triangle_step(tick: int, maximum: int) -> int:
    """Return 0..maximum..0, including both endpoints."""
    if maximum <= 0:
        return 0
    position = tick % (maximum * 2)
    return position if position <= maximum else maximum * 2 - position


def gauge_value_for_bars(lit: int, minimum: float, maximum: float) -> float:
    """Choose a value safely inside the firmware bucket for 0..9 lit bars."""
    lit = max(0, min(GAUGE_BAR_COUNT, lit))
    if lit == 0:
        return minimum
    if lit == GAUGE_BAR_COUNT:
        return maximum
    return minimum + (lit + 0.5) * (maximum - minimum) / GAUGE_BAR_COUNT


def indicator_mask(lit: int) -> int:
    """Fill physical indicator bits 0..23 from the start of the chain."""
    lit = max(0, min(INDICATOR_COUNT, lit))
    return (1 << lit) - 1 if lit else 0


def gauge_warning_mask(elapsed_seconds: float) -> int:
    """Blink all four gauge warnings for one second on, one second off."""
    return GAUGE_WARNING_MASK if int(max(0.0, elapsed_seconds)) % 2 == 0 else 0


class NcaLink:
    """Serial transport with protocol pacing and per-topic sequences."""

    def __init__(self, logger) -> None:
        self.port: serial.Serial | None = None
        self.logger = logger
        self.sequence: dict[int, int] = defaultdict(int)
        self.last_write = 0.0

    @property
    def connected(self) -> bool:
        return self.port is not None and self.port.is_open

    def open(self, name: str) -> None:
        self.close()
        if not name:
            raise ValueError("Select a serial port")
        self.port = serial.Serial(
            name, BAUD_RATE, bytesize=8, parity="N", stopbits=1,
            timeout=0, write_timeout=1,
        )
        self.last_write = 0.0

    def close(self) -> None:
        if self.port is not None and self.port.is_open:
            self.port.close()

    def send(self, destination: int, command: int, payload: bytes = b"",
             label: str = "frame") -> None:
        if not self.connected:
            raise RuntimeError("Serial port is not connected")
        delay = DEFAULT_PARCEL_DELAY - (time.monotonic() - self.last_write)
        if delay > 0.0:
            time.sleep(delay)
        frame = make_frame(destination, command, payload)
        self.port.write(frame)
        self.port.flush()
        self.last_write = time.monotonic()
        self.logger(
            f"TX {label}: payload={len(payload)} bytes, frame={frame.hex(' ')}"
        )

    def publish(self, topic: int, data: bytes) -> None:
        sequence = self.sequence[topic]
        self.sequence[topic] = (sequence + 1) & 0xFFFF
        payload = struct.pack("<BBH", PROTOCOL_VERSION, topic, sequence) + data
        self.send(BROADCAST, CMD_PUBLISH, payload, f"topic 0x{topic:02x}")

    def set_mode(self, mode: int) -> None:
        name = "interactive" if mode == MODE_INTERACTIVE else "raw"
        self.send(BROADCAST, CMD_SET_MODE, bytes((mode,)), f"mode {name}")

    def set_range(self, destination: int, minimum: float, maximum: float) -> None:
        payload = bytes((PROTOCOL_VERSION, 0))
        payload += bytes((CONFIG_MINIMUM_F32, 4)) + struct.pack("<f", minimum)
        payload += bytes((CONFIG_MAXIMUM_F32, 4)) + struct.pack("<f", maximum)
        self.send(destination, CMD_SET_CONFIG, payload, f"range device {destination}")


def value_spin(minimum: float, maximum: float, value: float,
               decimals: int = 1) -> QtWidgets.QDoubleSpinBox:
    widget = QtWidgets.QDoubleSpinBox()
    widget.setRange(minimum, maximum)
    widget.setDecimals(decimals)
    widget.setValue(value)
    widget.setKeyboardTracking(False)
    return widget


class MainWindow(QtWidgets.QWidget):
    def __init__(self) -> None:
        super().__init__()
        self.setWindowTitle("NCA Five-device Cluster Demo")
        self.resize(920, 760)
        self.link = NcaLink(self.log_message)
        self.demo_started = 0.0
        self.demo_ticks = 0
        self.warning_on = None

        self.port_combo = QtWidgets.QComboBox()
        self.refresh_button = QtWidgets.QPushButton("Refresh")
        self.connect_button = QtWidgets.QPushButton("Connect")
        connection = QtWidgets.QHBoxLayout()
        connection.addWidget(QtWidgets.QLabel("Port:"))
        connection.addWidget(self.port_combo, 1)
        connection.addWidget(self.refresh_button)
        connection.addWidget(self.connect_button)

        self.raw_button = QtWidgets.QPushButton("Raw mode")
        self.interactive_button = QtWidgets.QPushButton("Interactive mode")
        self.configure_button = QtWidgets.QPushButton("Configure default ranges")
        mode_row = QtWidgets.QHBoxLayout()
        mode_row.addWidget(self.raw_button)
        mode_row.addWidget(self.interactive_button)
        mode_row.addWidget(self.configure_button)

        self.rpm = value_spin(0, RPM_MAXIMUM, 0, 0)
        self.speed = value_spin(0, SPEED_MAXIMUM, 0)
        self.coolant = value_spin(COOLANT_MINIMUM, COOLANT_MAXIMUM,
                                  COOLANT_MINIMUM)
        self.fuel = value_spin(FUEL_MINIMUM, FUEL_MAXIMUM, FUEL_MINIMUM)
        self.indicators_low = QtWidgets.QLineEdit("00000000")
        self.indicators_high = QtWidgets.QLineEdit("00000000")

        telemetry = QtWidgets.QGroupBox("Five-device network — manual telemetry")
        telemetry_grid = QtWidgets.QGridLayout(telemetry)
        rows = (
            ("RPM", self.rpm, "Send RPM", self.send_rpm),
            ("Speed (km/h)", self.speed, "Send speed", self.send_speed),
            ("Coolant temperature (°C)", self.coolant, "Send coolant", self.send_coolant),
            ("Fuel level", self.fuel, "Send fuel", self.send_fuel),
        )
        for row, (label, editor, button_label, callback) in enumerate(rows):
            button = QtWidgets.QPushButton(button_label)
            button.clicked.connect(callback)
            telemetry_grid.addWidget(QtWidgets.QLabel(label), row, 0)
            telemetry_grid.addWidget(editor, row, 1)
            telemetry_grid.addWidget(button, row, 2)

        indicator_button = QtWidgets.QPushButton("Send indicators")
        indicator_button.clicked.connect(self.send_indicators)
        telemetry_grid.addWidget(QtWidgets.QLabel("Indicators low/high (hex)"), 4, 0)
        indicator_words = QtWidgets.QHBoxLayout()
        indicator_words.addWidget(self.indicators_low)
        indicator_words.addWidget(self.indicators_high)
        telemetry_grid.addLayout(indicator_words, 4, 1)
        telemetry_grid.addWidget(indicator_button, 4, 2)

        animation = QtWidgets.QGroupBox("Cluster animation state")
        animation_grid = QtWidgets.QGridLayout(animation)
        self.coolant_bars = self.add_progress(
            animation_grid, 0, "Coolant gauge", GAUGE_BAR_COUNT, " bars"
        )
        self.fuel_bars = self.add_progress(
            animation_grid, 1, "Fuel gauge", GAUGE_BAR_COUNT, " bars"
        )
        self.rpm_progress = self.add_progress(
            animation_grid, 2, "RPM gauge", RPM_MAXIMUM, " RPM"
        )
        self.speed_progress = self.add_progress(
            animation_grid, 3, "Speedometer", int(SPEED_MAXIMUM), " km/h"
        )
        self.indicator_progress = self.add_progress(
            animation_grid, 4, "VAZ indicators", INDICATOR_COUNT, " LEDs"
        )
        self.warning_status = QtWidgets.QLabel("OFF")
        animation_grid.addWidget(QtWidgets.QLabel("Gauge LED 16 warnings"), 5, 0)
        animation_grid.addWidget(self.warning_status, 5, 1, 1, 2)
        self.interval_spin = QtWidgets.QSpinBox()
        self.interval_spin.setRange(50, 1000)
        self.interval_spin.setValue(DEMO_INTERVAL_MS)
        self.interval_spin.setSuffix(" ms")
        animation_grid.addWidget(QtWidgets.QLabel("Animation tick"), 6, 0)
        animation_grid.addWidget(self.interval_spin, 6, 1)

        self.send_all_button = QtWidgets.QPushButton("Publish all values once")
        self.auto_button = QtWidgets.QPushButton("Start animated demo")
        self.auto_button.setCheckable(True)
        demo_row = QtWidgets.QHBoxLayout()
        demo_row.addWidget(self.send_all_button)
        demo_row.addWidget(self.auto_button)

        self.log = QtWidgets.QPlainTextEdit()
        self.log.setReadOnly(True)
        self.log.document().setMaximumBlockCount(500)

        layout = QtWidgets.QVBoxLayout(self)
        layout.addLayout(connection)
        layout.addLayout(mode_row)
        layout.addWidget(telemetry)
        layout.addWidget(animation)
        layout.addLayout(demo_row)
        layout.addWidget(QtWidgets.QLabel("Transmit log:"))
        layout.addWidget(self.log, 1)

        self.timer = QtCore.QTimer(self)
        self.timer.setInterval(DEMO_INTERVAL_MS)
        self.timer.timeout.connect(self.animate)
        self.refresh_button.clicked.connect(self.refresh_ports)
        self.connect_button.clicked.connect(self.toggle_connection)
        self.raw_button.clicked.connect(lambda: self.guarded(self.link.set_mode, MODE_RAW))
        self.interactive_button.clicked.connect(
            lambda: self.guarded(self.link.set_mode, MODE_INTERACTIVE)
        )
        self.configure_button.clicked.connect(self.configure_cluster)
        self.send_all_button.clicked.connect(self.publish_all)
        self.auto_button.toggled.connect(self.toggle_demo)
        self.interval_spin.valueChanged.connect(self.timer.setInterval)
        self.refresh_ports()

    @staticmethod
    def add_progress(layout: QtWidgets.QGridLayout, row: int, label: str,
                     maximum: int, suffix: str) -> QtWidgets.QProgressBar:
        progress = QtWidgets.QProgressBar()
        progress.setRange(0, maximum)
        progress.setFormat(f"%v / %m{suffix}")
        layout.addWidget(QtWidgets.QLabel(label), row, 0)
        layout.addWidget(progress, row, 1, 1, 2)
        return progress

    def log_message(self, message: str) -> None:
        if hasattr(self, "log"):
            self.log.appendPlainText(message)

    def guarded(self, callback, *args) -> bool:
        try:
            callback(*args)
            return True
        except Exception as error:
            self.log_message(f"Error: {error}")
            return False

    def refresh_ports(self) -> None:
        selected = self.port_combo.currentText()
        self.port_combo.clear()
        self.port_combo.addItems(port.device for port in serial.tools.list_ports.comports())
        if selected:
            self.port_combo.setCurrentText(selected)

    def toggle_connection(self) -> None:
        if self.link.connected:
            self.timer.stop()
            self.auto_button.setChecked(False)
            self.link.close()
            self.connect_button.setText("Connect")
            self.log_message("Disconnected")
        elif self.guarded(self.link.open, self.port_combo.currentText()):
            self.connect_button.setText("Disconnect")
            self.log_message(f"Connected at {BAUD_RATE} baud")

    def configure_cluster(self) -> None:
        self.guarded(self.configure_demo_cluster)

    def configure_demo_cluster(self) -> None:
        """Configure exactly the five devices used by this cluster demo."""
        self.link.set_mode(MODE_RAW)
        for address, minimum, maximum in (
            (DEVICE_COOLANT, COOLANT_MINIMUM, COOLANT_MAXIMUM),
            (DEVICE_SPEEDOMETER_BLOCK, 0.0, SPEED_MAXIMUM),
            (DEVICE_RPM, 0.0, float(RPM_MAXIMUM)),
            (DEVICE_FUEL, FUEL_MINIMUM, FUEL_MAXIMUM),
        ):
            self.link.set_range(address, minimum, maximum)
        self.link.set_mode(MODE_INTERACTIVE)

    def send_rpm(self) -> None:
        self.guarded(self.link.publish, TOPIC_RPM, struct.pack("<H", round(self.rpm.value())))

    def send_speed(self) -> None:
        self.guarded(self.link.publish, TOPIC_SPEED,
                     struct.pack("<H", round(self.speed.value() * 10.0)))

    def send_coolant(self) -> None:
        self.guarded(self.link.publish, TOPIC_COOLANT_TEMPERATURE,
                     signed_deci(self.coolant.value()))

    def send_fuel(self) -> None:
        self.guarded(self.link.publish, TOPIC_FUEL_LEVEL, signed_deci(self.fuel.value()))

    def send_indicators(self) -> None:
        try:
            low = int(self.indicators_low.text(), 16)
            high = int(self.indicators_high.text(), 16)
            if not 0 <= low <= 0xFFFFFFFF or not 0 <= high <= 0xFFFFFFFF:
                raise ValueError
        except ValueError:
            self.log_message("Error: indicator words must be 00000000..FFFFFFFF")
            return
        self.guarded(self.link.publish, TOPIC_INDICATORS, struct.pack("<II", low, high))

    def publish_all(self) -> None:
        self.send_rpm()
        self.send_speed()
        self.send_coolant()
        self.send_fuel()
        self.send_indicators()

    def toggle_demo(self, enabled: bool) -> None:
        if enabled and not self.link.connected:
            self.log_message("Error: connect a serial port before starting the demo")
            self.auto_button.setChecked(False)
            return
        if enabled:
            if not self.guarded(self.configure_demo_cluster):
                self.auto_button.setChecked(False)
                return
            self.demo_started = time.monotonic()
            self.demo_ticks = 0
            self.warning_on = None
            self.auto_button.setText("Stop animated demo")
            self.animate()
            if self.auto_button.isChecked():
                self.timer.start()
        else:
            self.timer.stop()
            self.auto_button.setText("Start animated demo")

    def animate(self) -> None:
        if not self.guarded(self.publish_demo_tick):
            self.auto_button.setChecked(False)

    def publish_demo_tick(self) -> None:
        """Advance all five cluster devices by one deterministic demo tick."""
        tick = self.demo_ticks
        sweep = triangle_step(tick, 80)
        rpm = round(sweep * RPM_MAXIMUM / 80)
        speed = sweep * SPEED_MAXIMUM / 80
        bars = triangle_step(tick // 4, GAUGE_BAR_COUNT)
        indicators = triangle_step(tick // 2, INDICATOR_COUNT)
        warning_mask = gauge_warning_mask(time.monotonic() - self.demo_started)
        warning_on = warning_mask != 0
        mask = indicator_mask(indicators)
        mask |= warning_mask

        coolant = gauge_value_for_bars(
            bars, COOLANT_MINIMUM, COOLANT_MAXIMUM
        )
        fuel = gauge_value_for_bars(bars, FUEL_MINIMUM, FUEL_MAXIMUM)

        self.rpm.setValue(rpm)
        self.speed.setValue(speed)
        self.coolant.setValue(coolant)
        self.fuel.setValue(fuel)
        self.indicators_low.setText(f"{mask:08X}")
        self.indicators_high.setText("00000000")
        self.rpm_progress.setValue(rpm)
        self.speed_progress.setValue(round(speed))
        self.coolant_bars.setValue(bars)
        self.fuel_bars.setValue(bars)
        self.indicator_progress.setValue(indicators)
        self.warning_status.setText("ON" if warning_on else "OFF")

        # RPM and speed move every tick. Gauges and indicator bits are sent
        # only when their visible state changes, keeping the UART load modest.
        self.link.publish(TOPIC_RPM, struct.pack("<H", rpm))
        self.link.publish(TOPIC_SPEED, struct.pack("<H", round(speed * 10.0)))
        if tick % 4 == 0:
            self.link.publish(TOPIC_COOLANT_TEMPERATURE, signed_deci(coolant))
            self.link.publish(TOPIC_FUEL_LEVEL, signed_deci(fuel))
        if tick % 2 == 0 or warning_on != self.warning_on:
            self.link.publish(TOPIC_INDICATORS, struct.pack("<II", mask, 0))
        self.warning_on = warning_on
        self.demo_ticks += 1

    def closeEvent(self, event) -> None:
        self.timer.stop()
        self.link.close()
        event.accept()


def main() -> None:
    application = QtWidgets.QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(application.exec_())


if __name__ == "__main__":
    main()
