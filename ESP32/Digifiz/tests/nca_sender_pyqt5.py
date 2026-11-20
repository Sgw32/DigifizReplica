#!/usr/bin/env python3
"""One-way UART controller for New Cluster Architecture LED devices."""

import sys
from dataclasses import dataclass

import serial
import serial.tools.list_ports
from PyQt5 import QtCore, QtGui, QtWidgets

SOF1 = 0xA5
BROADCAST_ADDRESS = 0x5A
BAUD = 250000

CMD_SET_PACKED_RANGE = 0x03
CMD_SET_RGB_ENABLED = 0x30
CMD_SET_RGB_ALL = 0x31
CMD_SET_RGB_RANGE = 0x32
CMD_SET_ADDRESS = 0x33

DEVICES = {
    "VAZ gauge (address 1)": (20, 1),
    "Speedometer block (address 2)": (56, 2),
    "Speedometer (address 3)": (42, 3),
    "VAZ indicators (address 4)": (24, 4),
    "RPM gauge (address 5)": (44, 5),
}


def xor_crc(data: bytes) -> int:
    result = 0
    for value in data:
        result ^= value
    return result


def valid_device_address(address: int) -> bool:
    return 1 <= address <= 0xFE and address != BROADCAST_ADDRESS


def pack_frame(destination: int, command: int, payload: bytes = b"") -> bytes:
    if destination != BROADCAST_ADDRESS and not valid_device_address(destination):
        raise ValueError("Address must be 1..254, excluding reserved broadcast 0x5A")
    if len(payload) > 0xFFFF:
        raise ValueError("Payload is too large")
    body = bytes((command, len(payload) & 0xFF, len(payload) >> 8)) + payload
    return bytes((SOF1, destination)) + body + bytes((xor_crc(body),))


def pack_bits(bits) -> bytes:
    packed = bytearray((len(bits) + 7) // 8)
    for index, enabled in enumerate(bits):
        if enabled:
            packed[index >> 3] |= 1 << (index & 7)
    return bytes(packed)


def packed_range_payload(offset: int, bits) -> bytes:
    bits = list(bits)
    count = len(bits)
    return bytes((offset & 0xFF, offset >> 8, count & 0xFF, count >> 8)) + pack_bits(bits)


def rgb_range_payload(offset: int, colors) -> bytes:
    colors = list(colors)
    payload = bytearray((offset & 0xFF, offset >> 8,
                         len(colors) & 0xFF, len(colors) >> 8))
    for red, green, blue in colors:
        payload.extend((red, green, blue))
    return bytes(payload)


def default_theme(count: int):
    """Match the firmware's MK2-v yellow-to-green NCA presets."""
    colors = []
    for index in range(count):
        hue = 64 + index * (100 - 64) // count
        saturation = 219
        value = 32
        region = hue // 43
        remainder = (hue - region * 43) * 6
        pale = value * (255 - saturation) >> 8
        falling = value * (255 - (saturation * remainder >> 8)) >> 8
        rising = value * (255 - (saturation * (255 - remainder) >> 8)) >> 8
        if region == 0:
            color = (value, rising, pale)
        elif region == 1:
            color = (falling, value, pale)
        elif region == 2:
            color = (pale, value, rising)
        elif region == 3:
            color = (pale, falling, value)
        elif region == 4:
            color = (rising, pale, value)
        else:
            color = (value, pale, falling)
        colors.append(color)
    return colors


@dataclass
class SerialLink:
    port: serial.Serial = None

    def open(self, name: str):
        self.close()
        self.port = serial.Serial(name, BAUD, timeout=0)

    def close(self):
        if self.port is not None and self.port.is_open:
            self.port.close()

    def send(self, frame: bytes):
        if self.port is None or not self.port.is_open:
            raise RuntimeError("Serial port is not connected")
        self.port.write(frame)


class MainWindow(QtWidgets.QWidget):
    def __init__(self):
        super().__init__()
        self.link = SerialLink()
        self.colors = []
        self.setWindowTitle("NCA Network Controller")
        self.resize(820, 720)

        self.port_combo = QtWidgets.QComboBox()
        self.refresh_button = QtWidgets.QPushButton("Refresh")
        self.connect_button = QtWidgets.QPushButton("Connect")
        self.device_combo = QtWidgets.QComboBox()
        self.device_combo.addItems(DEVICES)
        self.address_spin = QtWidgets.QSpinBox()
        self.address_spin.setRange(1, 254)
        self.broadcast_check = QtWidgets.QCheckBox("Broadcast")
        self.broadcast_check.setChecked(True)

        self.led_count_spin = QtWidgets.QSpinBox()
        self.led_count_spin.setRange(1, 56)
        self.segment_table = QtWidgets.QTableWidget()
        self.segment_table.setColumnCount(5)
        self.segment_table.setHorizontalHeaderLabels(("LED", "On", "R", "G", "B"))
        self.segment_table.horizontalHeader().setSectionResizeMode(
            QtWidgets.QHeaderView.Stretch
        )
        self.segment_table.setSelectionBehavior(QtWidgets.QAbstractItemView.SelectRows)

        self.all_on_button = QtWidgets.QPushButton("All on")
        self.all_off_button = QtWidgets.QPushButton("All off")
        self.send_segments_button = QtWidgets.QPushButton("Send segments")
        self.rgb_enabled_check = QtWidgets.QCheckBox("Use protocol RGB colors")
        self.send_rgb_mode_button = QtWidgets.QPushButton("Send RGB mode")
        self.pick_color_button = QtWidgets.QPushButton("Pick selected color")
        self.send_colors_button = QtWidgets.QPushButton("Send all colors")

        self.new_address_spin = QtWidgets.QSpinBox()
        self.new_address_spin.setRange(1, 254)
        self.change_address_button = QtWidgets.QPushButton("Change device address")

        self.log = QtWidgets.QPlainTextEdit()
        self.log.setReadOnly(True)

        connection = QtWidgets.QHBoxLayout()
        connection.addWidget(QtWidgets.QLabel("Port:"))
        connection.addWidget(self.port_combo, 1)
        connection.addWidget(self.refresh_button)
        connection.addWidget(self.connect_button)

        routing = QtWidgets.QGridLayout()
        routing.addWidget(QtWidgets.QLabel("Product preset:"), 0, 0)
        routing.addWidget(self.device_combo, 0, 1, 1, 3)
        routing.addWidget(QtWidgets.QLabel("Destination:"), 1, 0)
        routing.addWidget(self.address_spin, 1, 1)
        routing.addWidget(self.broadcast_check, 1, 2)
        routing.addWidget(QtWidgets.QLabel("LED count:"), 2, 0)
        routing.addWidget(self.led_count_spin, 2, 1)
        routing.addWidget(QtWidgets.QLabel("New address:"), 3, 0)
        routing.addWidget(self.new_address_spin, 3, 1)
        routing.addWidget(self.change_address_button, 3, 2, 1, 2)

        controls = QtWidgets.QGridLayout()
        controls.addWidget(self.all_on_button, 0, 0)
        controls.addWidget(self.all_off_button, 0, 1)
        controls.addWidget(self.send_segments_button, 0, 2)
        controls.addWidget(self.rgb_enabled_check, 1, 0, 1, 2)
        controls.addWidget(self.send_rgb_mode_button, 1, 2)
        controls.addWidget(self.pick_color_button, 2, 0, 1, 2)
        controls.addWidget(self.send_colors_button, 2, 2)

        layout = QtWidgets.QVBoxLayout(self)
        layout.addLayout(connection)
        layout.addLayout(routing)
        layout.addWidget(self.segment_table, 1)
        layout.addLayout(controls)
        layout.addWidget(QtWidgets.QLabel("Transmit log:"))
        layout.addWidget(self.log)

        self.refresh_button.clicked.connect(self.refresh_ports)
        self.connect_button.clicked.connect(self.toggle_connection)
        self.device_combo.currentTextChanged.connect(self.load_device)
        self.led_count_spin.valueChanged.connect(self.resize_table)
        self.all_on_button.clicked.connect(lambda: self.set_all_segments(True))
        self.all_off_button.clicked.connect(lambda: self.set_all_segments(False))
        self.send_segments_button.clicked.connect(self.send_segments)
        self.send_rgb_mode_button.clicked.connect(self.send_rgb_mode)
        self.pick_color_button.clicked.connect(self.pick_selected_color)
        self.send_colors_button.clicked.connect(self.send_colors)
        self.change_address_button.clicked.connect(self.change_address)
        self.segment_table.cellChanged.connect(self.table_color_changed)

        self.refresh_ports()
        self.load_device(self.device_combo.currentText())

    def log_message(self, message: str):
        self.log.appendPlainText(message)

    def refresh_ports(self):
        selected = self.port_combo.currentText()
        self.port_combo.clear()
        self.port_combo.addItems(port.device for port in serial.tools.list_ports.comports())
        if selected:
            self.port_combo.setCurrentText(selected)

    def toggle_connection(self):
        try:
            if self.link.port is not None and self.link.port.is_open:
                self.link.close()
                self.connect_button.setText("Connect")
                self.log_message("Disconnected")
            else:
                self.link.open(self.port_combo.currentText())
                self.connect_button.setText("Disconnect")
                self.log_message(f"Connected at {BAUD} baud")
        except Exception as error:
            self.log_message(f"Connection error: {error}")

    def load_device(self, name: str):
        count, address = DEVICES[name]
        self.address_spin.setValue(address)
        self.new_address_spin.setValue(address)
        self.led_count_spin.setValue(count)
        self.colors = default_theme(count)
        self.resize_table(count)

    def resize_table(self, count: int):
        old_colors = self.colors
        theme = default_theme(count)
        self.colors = [old_colors[i] if i < len(old_colors) else theme[i]
                       for i in range(count)]
        self.segment_table.blockSignals(True)
        self.segment_table.setRowCount(count)
        for row in range(count):
            index_item = QtWidgets.QTableWidgetItem(str(row))
            index_item.setFlags(index_item.flags() & ~QtCore.Qt.ItemIsEditable)
            self.segment_table.setItem(row, 0, index_item)
            checkbox = self.segment_table.item(row, 1)
            if checkbox is None:
                checkbox = QtWidgets.QTableWidgetItem()
                checkbox.setFlags(QtCore.Qt.ItemIsUserCheckable | QtCore.Qt.ItemIsEnabled)
                checkbox.setCheckState(QtCore.Qt.Unchecked)
                self.segment_table.setItem(row, 1, checkbox)
            self.set_color_cells(row, self.colors[row])
        self.segment_table.blockSignals(False)

    def set_color_cells(self, row: int, color):
        for column, value in enumerate(color, start=2):
            item = self.segment_table.item(row, column)
            if item is None:
                item = QtWidgets.QTableWidgetItem()
                self.segment_table.setItem(row, column, item)
            item.setText(str(value))
        swatch = QtGui.QColor(*color)
        self.segment_table.item(row, 0).setBackground(swatch)
        self.segment_table.item(row, 0).setForeground(
            QtGui.QColor("white") if swatch.value() < 128 else QtGui.QColor("black")
        )

    def table_color_changed(self, row: int, column: int):
        if column not in (2, 3, 4):
            return
        try:
            color = tuple(max(0, min(255, int(self.segment_table.item(row, col).text())))
                          for col in (2, 3, 4))
            self.colors[row] = color
            self.segment_table.blockSignals(True)
            self.set_color_cells(row, color)
            self.segment_table.blockSignals(False)
        except (TypeError, ValueError):
            self.log_message(f"Invalid RGB value in row {row}")

    def destination(self) -> int:
        return BROADCAST_ADDRESS if self.broadcast_check.isChecked() else self.address_spin.value()

    def transmit(self, command: int, payload: bytes, label: str, destination=None):
        try:
            target = self.destination() if destination is None else destination
            frame = pack_frame(target, command, payload)
            self.link.send(frame)
            shown = "broadcast" if target == BROADCAST_ADDRESS else str(target)
            self.log_message(
                f"TX {label} -> {shown}: payload={len(payload)} bytes, frame={frame.hex(' ')}"
            )
        except Exception as error:
            self.log_message(f"{label} error: {error}")

    def set_all_segments(self, enabled: bool):
        for row in range(self.segment_table.rowCount()):
            self.segment_table.item(row, 1).setCheckState(
                QtCore.Qt.Checked if enabled else QtCore.Qt.Unchecked
            )

    def send_segments(self):
        bits = [self.segment_table.item(row, 1).checkState() == QtCore.Qt.Checked
                for row in range(self.segment_table.rowCount())]
        self.transmit(CMD_SET_PACKED_RANGE, packed_range_payload(0, bits), "segments")

    def send_rgb_mode(self):
        self.transmit(CMD_SET_RGB_ENABLED,
                      bytes((int(self.rgb_enabled_check.isChecked()),)), "RGB mode")

    def pick_selected_color(self):
        rows = sorted({index.row() for index in self.segment_table.selectedIndexes()})
        if not rows:
            self.log_message("Select one or more rows before picking a color")
            return
        initial = QtGui.QColor(*self.colors[rows[0]])
        selected = QtWidgets.QColorDialog.getColor(initial, self, "Segment color")
        if selected.isValid():
            color = (selected.red(), selected.green(), selected.blue())
            self.segment_table.blockSignals(True)
            for row in rows:
                self.colors[row] = color
                self.set_color_cells(row, color)
            self.segment_table.blockSignals(False)

    def send_colors(self):
        payload = bytes(component for color in self.colors for component in color)
        self.transmit(CMD_SET_RGB_ALL, payload, "all RGB colors")

    def change_address(self):
        old_address = self.address_spin.value()
        new_address = self.new_address_spin.value()
        if not valid_device_address(new_address):
            self.log_message("New address cannot be 0, 0x5A, or 0xFF")
            return
        # Firmware intentionally accepts this command only as unicast.
        self.transmit(CMD_SET_ADDRESS, bytes((new_address,)),
                      "change address", destination=old_address)
        self.address_spin.setValue(new_address)
        self.broadcast_check.setChecked(False)

    def closeEvent(self, event):
        self.link.close()
        event.accept()


if __name__ == "__main__":
    application = QtWidgets.QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(application.exec_())

