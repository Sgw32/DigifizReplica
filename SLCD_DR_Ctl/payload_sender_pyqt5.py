#!/usr/bin/env python3
import sys
import random
from dataclasses import dataclass

import serial
import serial.tools.list_ports
from crc8 import crc8
from PyQt5 import QtCore, QtWidgets

SOF1 = 0xA5
SOF2 = 0x5A

CMD_SET_ALL = 0x01
CMD_SET_RANGE = 0x02
CMD_SET_PACKED_RANGE = 0x03
CMD_GET_INFO = 0x10
CMD_GET_RANGE = 0x11
CMD_SET_MODE = 0x20
CMD_INTERACTIVE_SET = 0x21

IF_SPEED = 0x01
IF_RPM = 0x02
IF_MFA_TYPE = 0x03
IF_MFA_NUMBER = 0x04
IF_MFA_CLOCK = 0x05
IF_FUEL = 0x06
IF_COOLANT = 0x07
IF_MILEAGE = 0x08
IF_CLOCK = 0x09
IF_DOT = 0x0A
IF_FLOAT_DOT = 0x0B
IF_REFUEL_SIGN = 0x0C
IF_MFA_BLOCK = 0x0D
IF_BACKLIGHT = 0x0E
IF_MAX_RPM = 0x0F

RSP_ACK = 0x80
RSP_NACK = 0x81
RSP_INFO = 0x90
RSP_RANGE = 0x91

BAUD = 250000
DEFAULT_PAYLOAD_BITS = 520


def pack_frame(cmd: int, payload: bytes = b"") -> bytes:
    header = bytes([cmd, len(payload) & 0xFF, (len(payload) >> 8) & 0xFF])
    c = crc8(header + payload)
    return bytes([SOF1, SOF2]) + header + payload + bytes([c])


def pack_bits(bits: bytes) -> bytes:
    packed = bytearray((len(bits) + 7) // 8)
    for i, bit in enumerate(bits):
        if bit:
            packed[i >> 3] |= 1 << (i & 0x07)
    return bytes(packed)


def make_packed_range_payload(offset: int, bits: bytes) -> bytes:
    count = len(bits)
    return bytes([
        offset & 0xFF,
        (offset >> 8) & 0xFF,
        count & 0xFF,
        (count >> 8) & 0xFF,
    ]) + pack_bits(bits)


def pack_interactive_fields(fields):
    payload = bytearray()
    for field, value in fields:
        value = int(value) & 0xFFFFFFFF
        payload.extend([field, value & 0xFF, (value >> 8) & 0xFF, (value >> 16) & 0xFF, (value >> 24) & 0xFF])
    return bytes(payload)


@dataclass
class Frame:
    cmd: int
    payload: bytes


class SerialClient:
    def __init__(self):
        self.ser = None

    def connect(self, port: str, baud: int = BAUD):
        self.ser = serial.Serial(port, baudrate=baud, timeout=0.7)

    def close(self):
        if self.ser and self.ser.is_open:
            self.ser.close()

    def send(self, cmd: int, payload: bytes = b""):
        if not self.ser or not self.ser.is_open:
            raise RuntimeError("Not connected")
        self.ser.write(pack_frame(cmd, payload))

    def read_frame(self) -> Frame:
        if not self.ser or not self.ser.is_open:
            raise RuntimeError("Not connected")

        while True:
            b = self.ser.read(1)
            if not b:
                raise TimeoutError("Timeout waiting for frame")

            if b[0] == SOF1:
                b2 = self.ser.read(1)
                if not b2:
                    raise TimeoutError("Timeout waiting SOF2")

                if b2[0] == SOF2:
                    break

        head = self.ser.read(3)
        if len(head) != 3:
            raise TimeoutError("Timeout waiting header")

        cmd = head[0]
        ln = head[1] | (head[2] << 8)

        payload = self.ser.read(ln)
        if len(payload) != ln:
            raise TimeoutError("Timeout waiting payload")

        crc = self.ser.read(1)
        if len(crc) != 1:
            raise TimeoutError("Timeout waiting CRC")

        calc = crc8(head + payload)
        if crc[0] != calc:
            raise ValueError(f"CRC mismatch: got {crc[0]:02X}, expected {calc:02X}")

        return Frame(cmd=cmd, payload=payload)


class MainWindow(QtWidgets.QWidget):
    def __init__(self):
        super().__init__()

        self.client = SerialClient()

        self.setWindowTitle("SLCD_DR_Ctl Payload Sender")
        self.resize(800, 500)

        self.port_combo = QtWidgets.QComboBox()
        self.refresh_btn = QtWidgets.QPushButton("Refresh")
        self.connect_btn = QtWidgets.QPushButton("Connect")

        self.payload_edit = QtWidgets.QPlainTextEdit()
        self.payload_edit.setPlaceholderText(
            "Enter payload bytes as 0/1, e.g. 1,0,1,1 or whitespace-separated"
        )

        self.offset_spin = QtWidgets.QSpinBox()
        self.offset_spin.setRange(0, 520)

        self.count_spin = QtWidgets.QSpinBox()
        self.count_spin.setRange(1, 520)

        self.send_all_btn = QtWidgets.QPushButton("Send SET_ALL")
        self.send_range_btn = QtWidgets.QPushButton("Send SET_RANGE")
        self.send_packed_all_btn = QtWidgets.QPushButton("Send PACKED_ALL")
        self.send_packed_range_btn = QtWidgets.QPushButton("Send PACKED_RANGE")
        self.info_btn = QtWidgets.QPushButton("GET_INFO")
        self.get_range_btn = QtWidgets.QPushButton("GET_RANGE")

        self.random_range_btn = QtWidgets.QPushButton("Randomize PACKED_RANGE: OFF")
        self.random_range_btn.setCheckable(True)

        self.interactive_btn = QtWidgets.QPushButton("Enter interactive mode")
        self.speed_spin = QtWidgets.QSpinBox(); self.speed_spin.setRange(0, 999)
        self.rpm_spin = QtWidgets.QSpinBox(); self.rpm_spin.setRange(0, 12000)
        self.max_rpm_spin = QtWidgets.QSpinBox(); self.max_rpm_spin.setRange(1, 12000); self.max_rpm_spin.setValue(7000)
        self.fuel_spin = QtWidgets.QSpinBox(); self.fuel_spin.setRange(0, 99)
        self.coolant_spin = QtWidgets.QSpinBox(); self.coolant_spin.setRange(0, 20)
        self.mileage_spin = QtWidgets.QSpinBox(); self.mileage_spin.setRange(0, 999999)
        self.mfa_type_spin = QtWidgets.QSpinBox(); self.mfa_type_spin.setRange(0, 6)
        self.mfa_number_spin = QtWidgets.QSpinBox(); self.mfa_number_spin.setRange(-9999, 9999)
        self.clock_hour_spin = QtWidgets.QSpinBox(); self.clock_hour_spin.setRange(0, 23)
        self.clock_min_spin = QtWidgets.QSpinBox(); self.clock_min_spin.setRange(0, 59)
        self.mfa_hour_spin = QtWidgets.QSpinBox(); self.mfa_hour_spin.setRange(0, 99)
        self.mfa_min_spin = QtWidgets.QSpinBox(); self.mfa_min_spin.setRange(0, 99)
        self.dot_chk = QtWidgets.QCheckBox("Clock/MFA dot")
        self.float_dot_chk = QtWidgets.QCheckBox("MFA float dot")
        self.refuel_chk = QtWidgets.QCheckBox("Refuel sign")
        self.send_interactive_btn = QtWidgets.QPushButton("Send interactive values")

        self.random_timer = QtCore.QTimer(self)
        self.random_timer.setInterval(1000 // 32)
        self.random_timer.timeout.connect(self.randomize_send_range)

        self.log = QtWidgets.QPlainTextEdit()
        self.log.setReadOnly(True)

        top = QtWidgets.QHBoxLayout()
        top.addWidget(QtWidgets.QLabel("Serial Port:"))
        top.addWidget(self.port_combo)
        top.addWidget(self.refresh_btn)
        top.addWidget(self.connect_btn)

        range_row = QtWidgets.QHBoxLayout()
        range_row.addWidget(QtWidgets.QLabel("Offset:"))
        range_row.addWidget(self.offset_spin)
        range_row.addWidget(QtWidgets.QLabel("Count:"))
        range_row.addWidget(self.count_spin)
        range_row.addStretch(1)

        btn_row = QtWidgets.QHBoxLayout()
        btn_row.addWidget(self.send_all_btn)
        btn_row.addWidget(self.send_range_btn)
        btn_row.addWidget(self.send_packed_all_btn)
        btn_row.addWidget(self.send_packed_range_btn)
        btn_row.addWidget(self.info_btn)
        btn_row.addWidget(self.get_range_btn)
        btn_row.addWidget(self.random_range_btn)

        lay = QtWidgets.QVBoxLayout(self)
        lay.addLayout(top)
        lay.addWidget(QtWidgets.QLabel("Payload bytes:"))
        lay.addWidget(self.payload_edit, 1)
        lay.addLayout(range_row)
        lay.addLayout(btn_row)

        interactive_grid = QtWidgets.QGridLayout()
        interactive_grid.addWidget(self.interactive_btn, 0, 0, 1, 2)
        labels_widgets = [
            ("Speed", self.speed_spin), ("RPM", self.rpm_spin), ("Max RPM", self.max_rpm_spin),
            ("Fuel", self.fuel_spin), ("Coolant", self.coolant_spin), ("Mileage", self.mileage_spin),
            ("MFA type", self.mfa_type_spin), ("MFA number", self.mfa_number_spin),
            ("Clock hour", self.clock_hour_spin), ("Clock min", self.clock_min_spin),
            ("MFA hour", self.mfa_hour_spin), ("MFA min", self.mfa_min_spin),
        ]
        for idx, (label, widget) in enumerate(labels_widgets, start=1):
            row = (idx + 1) // 2
            col = 0 if idx % 2 else 2
            interactive_grid.addWidget(QtWidgets.QLabel(label + ":"), row, col)
            interactive_grid.addWidget(widget, row, col + 1)
        interactive_grid.addWidget(self.dot_chk, 7, 0, 1, 2)
        interactive_grid.addWidget(self.float_dot_chk, 7, 2, 1, 2)
        interactive_grid.addWidget(self.refuel_chk, 8, 0, 1, 2)
        interactive_grid.addWidget(self.send_interactive_btn, 8, 2, 1, 2)
        lay.addWidget(QtWidgets.QLabel("Interactive compressed controls:"))
        lay.addLayout(interactive_grid)

        lay.addWidget(QtWidgets.QLabel("Log:"))
        lay.addWidget(self.log, 1)

        self.refresh_btn.clicked.connect(self.refresh_ports)
        self.connect_btn.clicked.connect(self.toggle_connect)
        self.send_all_btn.clicked.connect(self.send_all)
        self.send_range_btn.clicked.connect(self.send_range)
        self.send_packed_all_btn.clicked.connect(self.send_packed_all)
        self.send_packed_range_btn.clicked.connect(self.send_packed_range)
        self.info_btn.clicked.connect(self.get_info)
        self.get_range_btn.clicked.connect(self.get_range)
        self.random_range_btn.toggled.connect(self.toggle_random_range)
        self.interactive_btn.clicked.connect(self.enter_interactive_mode)
        self.send_interactive_btn.clicked.connect(self.send_interactive_values)

        self.refresh_ports()

    def log_msg(self, txt: str):
        self.log.appendPlainText(txt)

    def refresh_ports(self):
        self.port_combo.clear()
        for p in serial.tools.list_ports.comports():
            self.port_combo.addItem(p.device)

    def toggle_connect(self):
        if self.client.ser and self.client.ser.is_open:
            self.random_range_btn.setChecked(False)
            self.client.close()
            self.connect_btn.setText("Connect")
            self.log_msg("Disconnected")
            return

        port = self.port_combo.currentText().strip()
        if not port:
            self.log_msg("No serial port selected")
            return

        try:
            self.client.connect(port, BAUD)
            self.connect_btn.setText("Disconnect")
            self.log_msg(f"Connected to {port} @ " + str(BAUD))
        except Exception as e:
            self.log_msg(f"Connect error: {e}")

    def parse_payload_text(self) -> bytes:
        text = self.payload_edit.toPlainText().replace(",", " ").split()
        out = []

        for token in text:
            v = int(token, 0)
            if v not in (0, 1):
                raise ValueError("Only 0 or 1 payload values are allowed")
            out.append(v)

        return bytes(out)

    def transact(self, cmd: int, payload: bytes = b"") -> Frame:
        self.client.send(cmd, payload)
        fr = self.client.read_frame()
        self.log_msg(f"RX cmd=0x{fr.cmd:02X}, len={len(fr.payload)}")
        return fr

    def send_command(self, cmd: int, payload: bytes = b"", description: str = "command"):
        self.client.send(cmd, payload)
        self.log_msg(f"TX {description}: cmd=0x{cmd:02X}, payload_len={len(payload)}")

    def send_all(self):
        try:
            payload = self.parse_payload_text()
            self.send_command(CMD_SET_ALL, payload, "SET_ALL")

        except Exception as e:
            self.log_msg(f"SET_ALL error: {e}")

    def send_range(self):
        try:
            data = self.parse_payload_text()
            offset = self.offset_spin.value()
            count = len(data)

            payload = bytes([
                offset & 0xFF,
                (offset >> 8) & 0xFF,
                count & 0xFF,
                (count >> 8) & 0xFF,
            ]) + data

            self.send_command(CMD_SET_RANGE, payload, "SET_RANGE")

        except Exception as e:
            self.log_msg(f"SET_RANGE error: {e}")

    def send_packed_all(self):
        try:
            data = self.parse_payload_text()
            padded = data[:DEFAULT_PAYLOAD_BITS].ljust(DEFAULT_PAYLOAD_BITS, b"\x00")
            payload = make_packed_range_payload(0, padded)
            self.send_command(CMD_SET_PACKED_RANGE, payload, f"PACKED_ALL bits={len(padded)}")

        except Exception as e:
            self.log_msg(f"PACKED_ALL error: {e}")

    def send_packed_range(self):
        try:
            data = self.parse_payload_text()
            offset = self.offset_spin.value()
            payload = make_packed_range_payload(offset, data)
            self.send_command(CMD_SET_PACKED_RANGE, payload, f"PACKED_RANGE offset={offset}, bits={len(data)}")

        except Exception as e:
            self.log_msg(f"PACKED_RANGE error: {e}")

    def get_info(self):
        try:
            fr = self.transact(CMD_GET_INFO)

            if fr.cmd == RSP_INFO and len(fr.payload) == 2:
                size = fr.payload[0] | (fr.payload[1] << 8)
                self.log_msg(f"Payload size: {size}")
            else:
                self.log_msg(
                    f"Unexpected response: cmd=0x{fr.cmd:02X}, payload={fr.payload.hex()}"
                )

        except Exception as e:
            self.log_msg(f"GET_INFO error: {e}")

    def get_range(self):
        try:
            offset = self.offset_spin.value()
            count = self.count_spin.value()

            payload = bytes([
                offset & 0xFF,
                (offset >> 8) & 0xFF,
                count & 0xFF,
                (count >> 8) & 0xFF,
            ])

            fr = self.transact(CMD_GET_RANGE, payload)

            if fr.cmd == RSP_RANGE:
                txt = ",".join(str(x) for x in fr.payload)
                self.payload_edit.setPlainText(txt)
                self.log_msg(f"Received {len(fr.payload)} bytes")
            else:
                self.log_msg(f"Unexpected response: 0x{fr.cmd:02X}")

        except Exception as e:
            self.log_msg(f"GET_RANGE error: {e}")

    def toggle_random_range(self, enabled: bool):
        if enabled:
            if not self.client.ser or not self.client.ser.is_open:
                self.log_msg("Randomize error: not connected")
                self.random_range_btn.setChecked(False)
                return

            self.random_range_btn.setText("Randomize PACKED_RANGE: ON")
            self.random_timer.start()
            self.log_msg("Randomize PACKED_RANGE started @ 32 Hz")
        else:
            self.random_timer.stop()
            self.random_range_btn.setText("Randomize PACKED_RANGE: OFF")
            self.log_msg("Randomize PACKED_RANGE stopped")

    def randomize_send_range(self):
        try:
            if not self.client.ser or not self.client.ser.is_open:
                raise RuntimeError("Not connected")

            offset = self.offset_spin.value()
            count = self.count_spin.value()

            data = bytes(random.getrandbits(1) for _ in range(count))

            payload = make_packed_range_payload(offset, data)

            self.client.send(CMD_SET_PACKED_RANGE, payload)
            self.log_msg(
                "Random PACKED_RANGE TX "
                f"offset={offset}, count={count}, payload_bytes={len(payload)}"
            )

        except Exception as e:
            self.log_msg(f"Random PACKED_RANGE error: {e}")
            self.random_range_btn.setChecked(False)


    def enter_interactive_mode(self):
        try:
            self.send_command(CMD_SET_MODE, b"\x01", "enter interactive mode")
        except Exception as e:
            self.log_msg(f"Interactive mode error: {e}")

    def send_interactive_values(self):
        try:
            clock_value = (self.clock_hour_spin.value() << 8) | self.clock_min_spin.value()
            mfa_clock_value = (self.mfa_hour_spin.value() << 8) | self.mfa_min_spin.value()
            fields = [
                (IF_MAX_RPM, self.max_rpm_spin.value()),
                (IF_SPEED, self.speed_spin.value()),
                (IF_RPM, self.rpm_spin.value()),
                (IF_FUEL, self.fuel_spin.value()),
                (IF_COOLANT, self.coolant_spin.value()),
                (IF_MILEAGE, self.mileage_spin.value()),
                (IF_MFA_TYPE, self.mfa_type_spin.value()),
                (IF_MFA_NUMBER, self.mfa_number_spin.value()),
                (IF_CLOCK, clock_value),
                (IF_MFA_CLOCK, mfa_clock_value),
                (IF_DOT, int(self.dot_chk.isChecked())),
                (IF_FLOAT_DOT, int(self.float_dot_chk.isChecked())),
                (IF_REFUEL_SIGN, int(self.refuel_chk.isChecked())),
            ]
            payload = pack_interactive_fields(fields)
            self.send_command(CMD_INTERACTIVE_SET, payload, "interactive compressed update")
        except Exception as e:
            self.log_msg(f"Interactive update error: {e}")

    def closeEvent(self, event):
        self.random_timer.stop()
        self.client.close()
        event.accept()


if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    w = MainWindow()
    w.show()
    sys.exit(app.exec_())