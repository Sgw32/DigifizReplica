# Pulse capture and filtering tests

Run the host waveform tests from the repository root:

```sh
cc -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=address,undefined \
  -I ESP32/Digifiz/main ESP32/Digifiz/tests/pulse_filters_test.c \
  -lm -o /tmp/digifiz-pulse-tests
/tmp/digifiz-pulse-tests
```

The tests exercise the same integer RC filter, threshold detector, period
window, speed edge qualifier, median and cumulative counter window used in
the firmware. They include a waveform resembling the supplied oscilloscope
trace (68.49 Hz negative pulses with additional short low spikes), inverted
polarity, 500–8000 RPM at two pulses per revolution, acceleration/deceleration,
stop/restart, short high/low speed glitches, low speed, median startup, count
wrap and delayed counter sampling. These tests do not validate electrical
levels or interrupt latency on a board.

# RPM algorithm selection

Set **RPM algorithm** to **3** (`glitchanalog`) and reboot. Modes 0–2 and the
existing default are unchanged. Both Next and ReFIZ expose mode 3.

This mode models an RC circuit with a **400 microsecond time constant**
(approximately 398 Hz corner frequency), sampled every **25 microseconds**.
Its fixed-point update uses `1 - exp(-25/400)`. A Schmitt threshold at 45%/55%
turns the filtered voltage into pulses. A fully charged signal must stay low
for approximately 319 microseconds to cross the lower threshold; this is an
RC response, rather than a fixed 400 microsecond pulse-width gate. Closely
spaced glitches accumulate charge/discharge just as they would in an RC.

Frequency comes from rising threshold crossings, averaging the most recent
eight periods and updating after each new period. Startup uses available
periods rather than zero-filled samples. Existing Hz-to-RPM calibration and
the one-second no-signal timeout still apply. After a stop, the old period
window is discarded. This streaming estimator does not wait for an entire
eight-period batch and does not perform an FFT.

A separate GPIO edge handler and GPTimer sampling callback are installed at
boot for this mode. RC arithmetic in the interrupt uses only integers. The
25 microsecond timer creates 40,000 sample interrupts per second; verify
latency and CPU load with Wi-Fi/BLE enabled on the target board. Pulses close
to the sampling period or RC threshold cannot be resolved reliably.

# Speed capture

Stable and filtered speed modes qualify both high and low phases at 50 us.
Every accepted pulse generates a period; filtered mode then applies an
eight-sample median to valid periods. There is no extra eight-pulse batch
average and no zero padding on startup. Stable mode bypasses the median.
When both flags are enabled, stable mode takes precedence. Mode changes
require a reboot, matching the handler selected at initialization.

A one-item ISR mailbox keeps the newest period when the consumer is busy;
intermediate updates may be coalesced, but old queued readings cannot hide
the latest speed. Stale samples and pre-stop median history are discarded.

When both input flags are off, PCNT counts continuously and uses overflow
accumulation. Its glitch filter now honors `SPD_M_FILTER_LENGTH` (1023 APB
cycles, approximately 12.8 us at 80 MHz). Frequency is computed over the
configured rolling window using actual timestamps, including when the
measurement period and window differ. Counting is never cleared between
readings. The history spans the requested window to sample-period precision,
using the nearest sample at or before its boundary. Startup uses the
available elapsed interval. Its freshness timeout allows for the configured
counter update period.

For firmware validation, use ESP-IDF **v5.2.1**, target **esp32s3**, with the
desired `setup.h` variant selected. Build with `idf.py build`, or use
`idf.py reconfigure` followed by `ninja -C build -j 4` to limit parallelism.
# Speed LPF tests

From the repository root:

```sh
cc -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=address,undefined \
  -I ESP32/Digifiz/main ESP32/Digifiz/tests/speed_filter_test.c \
  -lm -o /tmp/digifiz-speed-filter-test
/tmp/digifiz-speed-filter-test
```

## ReFIZ full-payload diagnostics

```sh
cc -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=address,undefined \
  -I ESP32/Digifiz/main ESP32/Digifiz/tests/refiz_payload_test.c \
  -o /tmp/refiz-payload-test
/tmp/refiz-payload-test
```

The editor covers **all 520 raw payload bits (65 bytes)**, including control
and configuration bits. Bit labels show the raw bit number followed by
`byte.bit`; bytes are packed least-significant-bit first. The former logical
LCD segment 0 corresponds to raw payload bit 3. Every bit, including 0–2 and
519, can be changed. The SLCD controller may interpret some positions as
configuration/reserved data rather than visible LCD segments.

On opening the page, the editor reads the device state and firmware default
pattern. **Enable diagnostic mode** immediately starts diagnostics with the
ESP32 sender's original default payload, and the checkboxes reflect those
bytes. Edit the checkboxes and use **Apply payload** to transmit the complete
65-byte pattern. **Restore defaults** stages the device-provided default for
the next Apply; **Reload from device** reads the current state without resetting
an active custom pattern. Re-enabling diagnostics resets to defaults again.

Diagnostic UART frames use the existing `SET_PACKED_RANGE` command (`0x03`),
offset 0, bit count 520 and the existing XOR checksum. Snapshotting under a
lock prevents partially updated frames while the HTTP task changes settings.
Disabling diagnostics sends a full restored normal frame, including the upper
configuration bytes, before resuming the original 264-bit normal updates. The
first frame after startup also synchronizes the complete controller payload.

`GET /refiz_diagnostic` returns `enabled`, `bit_count`, `payload` (65 bytes),
and `default_payload` (65 bytes). POST accepts:

```json
{"enabled": true, "reset": true}
```

or `{"enabled": true, "payload": [...]}` with exactly 65 integer bytes in
0–255. `{"enabled": false}` resumes normal operation. Responses return the
same authoritative state as GET. Invalid sizes, non-integers, out-of-range
bytes, conflicting payload/reset options, and oversized requests are rejected
before changing the diagnostic state. The legacy `segments` array (logical
indexes 0–260) remains accepted for cached older pages.

Host tests exercise each of the 520 bits through the actual packed-frame
builder, full on/off masks, checksums, defaults, transitions and restoration
of upper bytes. Browser tests cover default checkbox initialization, lower and
upper bit edits, reactivation, reset/reload, select/clear all and failed-request
retries. Physical UART/SLCD behavior still requires hardware verification.

## Next block color alerts

```sh
cc -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=address,undefined \
  -I ESP32/Digifiz/main -I ESP32/Digifiz/components/xparam \
  ESP32/Digifiz/tests/block_color_alert_test.c -lm -o /tmp/block-color-alert-test
/tmp/block-color-alert-test
```

The production alert engine is tested for all five thresholds and recovery
boundaries, invalid/hidden inputs, synchronized phases, live settings, clock
rollover, restoration of per-LED theme colors and physical LED ranges. These
tests also instantiate the actual new parameter definitions and check defaults.

Next exposes a named **color mode** choice for each block: **No change**,
**Blinking**, or **Constant color**. Internally 0 means no change, 1 means
constant and 2 means blinking, preserving the original saved speed-color
enable value of 1. The existing speed threshold and RGB storage keys remain.
Other blocks default to no change; an existing enabled speed alert stays enabled.

| Block | Activation | Default threshold | Default hysteresis |
| --- | --- | --- | --- |
| RPM | Above threshold | 6000 RPM | 200 RPM |
| Speed | Above threshold | 120 displayed km/h or mph | 5 displayed units |
| Fuel | Below threshold | 10 liters | 2 liters |
| Coolant | Above threshold | 110 degrees Celsius | 5 degrees Celsius |
| Oil on MFA | Above threshold | 130 degrees Celsius | 5 degrees Celsius |

Activation is strict (`>` or `<`). High alerts clear at or below threshold
minus hysteresis; low fuel clears at or above threshold plus hysteresis.
Temperature limits always use Celsius and fuel always uses liters, independently
of Fahrenheit/Kelvin/gallons display options. Each block has its own RGB triplet,
defaulting to red. Changes take effect while running.

**Color alert blink period ms** is one shared full cycle, defaulting to 1000 ms:
500 ms in the selected/theme color and 500 ms in the alert color. All blocks
join the same phase, even if they trigger at different times. Constant mode
uses the alert RGB throughout. The overlay also works over LED effects and
does not change which pixels are lit or replace stored/custom color schemes.

Oil coloring is limited to the visible oil-temperature MFA page; unrelated MFA
pages, clock digits and the oil-pressure icon are unaffected. Faulty or invalid
temperature/fuel readings clear their color alerts. Speed alerts are suppressed
while gear glyphs are displayed; check-engine speed warnings take priority.

With a new fuel/coolant color mode enabled, the corresponding legacy low-fuel
or healthy-sensor overtemperature on/off blink is suppressed so the color
alternation stays visible. Legacy behavior remains when color mode is off;
sensor-fault and check-engine indications still operate.

The firmware renders alerts at its display-task rate; board testing is still
needed to verify LED appearance and electrical behavior.

## Web interface regression tests

Requires Node.js 20+ and Chromium. From the repository root:

```sh
cd ESP32/Digifiz/tests/web
npm ci
npm test
```

The tests use `/usr/bin/chromium` by default; set `CHROMIUM_PATH` for another
Chromium installation. They execute the actual embedded HTML in a headless
browser with mocked device HTTP endpoints and WebSocket connections. No
ESP32 or external server is required.

Coverage includes pending numeric/text/checkbox inputs, edits made during
an in-flight refresh, focus and selection, imported drafts, rejected writes
and partial bulk updates. Automatic refreshes update the loaded-value column
while preserving inputs. Explicit **Load Parameters** still reloads inputs
from the device.

The tests build both variant-specific pages with the production page generator
and check their gzip payloads. Next contains no Diagnostics markup or script;
ReFIZ retains it and can edit/apply all 520 payload bits, clear and disable the
pattern. Named color-mode choices and draft preservation are also exercised.

Color tests insert above and below the first, middle and last rows, insert
after moving a row, check the order submitted to the device, and verify
append/move/delete controls and the empty-table state.

Both station and fallback-AP HTTP servers now reserve 16 URI slots and a 20 KiB
stack. ReFIZ registers ten routes (eight application routes plus two Wi-Fi
routes), exceeding ESP-IDF's default eight-slot capacity. Next needs eight;
both have headroom and reject incomplete server registration rather than
silently serving a partial route set. Hardware HTTP/diagnostic behavior cannot
be exercised by the host browser mocks.

These tests use the production speed LPF helper and verify the step response,
decay to zero, legacy saved settings, coefficient limits, live changes and
noise attenuation. They do not require ESP32 hardware.

## Speed LPF settings

ESP32 uses the same first-order update as the A2560 speed filter and the RPM
LPF: `filtered += (speed - filtered) * K / 1000`. The filter runs once per
32 Hz display task cycle, before the display update divider. It applies to
both measured and RPM-emulated speed. The display test ramp bypasses it.
Mileage and gear estimation continue to use the unfiltered speed.

`speedFilterK` defaults to **100** for both Digifiz Next and ReFIZ. At 32 Hz,
this reaches 95% of a speed change in about **0.91 seconds**, and 96.6% in one
second. The exponential time constant (63.2% response) is about **0.30 seconds**;
the approximately one-second figure describes settling time. This preserves
the A2560/RPM coefficient scale rather than redefining `K` as a duration.

Higher values respond faster: **1000** follows the input immediately. Values
above 1000 are clamped. A saved value of **0**, the previously unused ESP32
default, uses 100 so an upgrade does not freeze the display. Zero no longer
means a frozen output; use 1000 to bypass smoothing. Coefficient changes are
applied on the next task cycle, without a reboot.

Both the parameter UI and legacy `PARAMETER_SPEED_FILTER` protocol setter
control the coefficient. Protocol reads preserve the full 0–1000 range.
