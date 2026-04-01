#!/usr/bin/env python3
import sys
from dataclasses import dataclass

import serial
import serial.tools.list_ports
from PyQt5 import QtCore, QtWidgets

SOF1 = 0xA5
SOF2 = 0x5A

CMD_SET_ALL = 0x01
CMD_SET_RANGE = 0x02
CMD_GET_INFO = 0x10
CMD_GET_RANGE = 0x11

RSP_ACK = 0x80
RSP_NACK = 0x81
RSP_INFO = 0x90
RSP_RANGE = 0x91


def crc8(data: bytes) -> int:
    c = 0
    for b in data:
        c ^= b
    return c


def pack_frame(cmd: int, payload: bytes = b"") -> bytes:
    header = bytes([cmd, len(payload) & 0xFF, (len(payload) >> 8) & 0xFF])
    c = crc8(header + payload)
    return bytes([SOF1, SOF2]) + header + payload + bytes([c])


@dataclass
class Frame:
    cmd: int
    payload: bytes


class SerialClient:
    def __init__(self):
        self.ser = None

    def connect(self, port: str, baud: int = 115200):
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
        self.payload_edit.setPlaceholderText("Enter payload bytes as 0/1, e.g. 1,0,1,1 or whitespace-separated")

        self.offset_spin = QtWidgets.QSpinBox()
        self.offset_spin.setRange(0, 520)
        self.count_spin = QtWidgets.QSpinBox()
        self.count_spin.setRange(1, 520)

        self.send_all_btn = QtWidgets.QPushButton("Send SET_ALL")
        self.send_range_btn = QtWidgets.QPushButton("Send SET_RANGE")
        self.info_btn = QtWidgets.QPushButton("GET_INFO")
        self.get_range_btn = QtWidgets.QPushButton("GET_RANGE")

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
        btn_row.addWidget(self.info_btn)
        btn_row.addWidget(self.get_range_btn)

        lay = QtWidgets.QVBoxLayout(self)
        lay.addLayout(top)
        lay.addWidget(QtWidgets.QLabel("Payload bytes:"))
        lay.addWidget(self.payload_edit, 1)
        lay.addLayout(range_row)
        lay.addLayout(btn_row)
        lay.addWidget(QtWidgets.QLabel("Log:"))
        lay.addWidget(self.log, 1)

        self.refresh_btn.clicked.connect(self.refresh_ports)
        self.connect_btn.clicked.connect(self.toggle_connect)
        self.send_all_btn.clicked.connect(self.send_all)
        self.send_range_btn.clicked.connect(self.send_range)
        self.info_btn.clicked.connect(self.get_info)
        self.get_range_btn.clicked.connect(self.get_range)

        self.refresh_ports()

    def log_msg(self, txt: str):
        self.log.appendPlainText(txt)

    def refresh_ports(self):
        self.port_combo.clear()
        for p in serial.tools.list_ports.comports():
            self.port_combo.addItem(p.device)

    def toggle_connect(self):
        if self.client.ser and self.client.ser.is_open:
            self.client.close()
            self.connect_btn.setText("Connect")
            self.log_msg("Disconnected")
            return

        port = self.port_combo.currentText().strip()
        if not port:
            self.log_msg("No serial port selected")
            return
        try:
            self.client.connect(port, 115200)
            self.connect_btn.setText("Disconnect")
            self.log_msg(f"Connected to {port} @ 115200")
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

    def send_all(self):
        try:
            payload = self.parse_payload_text()
            fr = self.transact(CMD_SET_ALL, payload)
            if fr.cmd == RSP_ACK:
                self.log_msg("SET_ALL ACK")
            else:
                self.log_msg(f"Unexpected response: 0x{fr.cmd:02X}")
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
            fr = self.transact(CMD_SET_RANGE, payload)
            if fr.cmd == RSP_ACK:
                self.log_msg("SET_RANGE ACK")
            else:
                self.log_msg(f"Unexpected response: 0x{fr.cmd:02X}")
        except Exception as e:
            self.log_msg(f"SET_RANGE error: {e}")

    def get_info(self):
        try:
            fr = self.transact(CMD_GET_INFO)
            if fr.cmd == RSP_INFO and len(fr.payload) == 2:
                size = fr.payload[0] | (fr.payload[1] << 8)
                self.log_msg(f"Payload size: {size}")
            else:
                self.log_msg(f"Unexpected response: cmd=0x{fr.cmd:02X}, payload={fr.payload.hex()}")
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


if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    w = MainWindow()
    w.show()
    sys.exit(app.exec_())
