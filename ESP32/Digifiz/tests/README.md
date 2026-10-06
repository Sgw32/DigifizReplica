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
