# Speed LPF tests

From the repository root:

```sh
cc -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=address,undefined \
  -I ESP32/Digifiz/main ESP32/Digifiz/tests/speed_filter_test.c \
  -lm -o /tmp/digifiz-speed-filter-test
/tmp/digifiz-speed-filter-test
```

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
