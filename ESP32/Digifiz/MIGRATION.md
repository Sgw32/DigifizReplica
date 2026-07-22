# Remote migration to the 8 MB partition layout

This procedure replaces the deployed factory/OTA partition table without opening the
device. It is deliberately a **two-firmware process**: first install the small migration
application using the existing web updater, then upload the normal expanded Digifiz
application from the migration application's page.

> **Warning:** interruption while the partition-table sector is being written can require
> recovery over USB. Test the exact binaries on development units first, use stable power,
> and do not deploy a migration binary built from a different partition table.

## Layout and safety strategy

The migration application is initially built with the old layout. On its first boot it:

1. Detects the old table by finding the `factory` partition.
2. Checks whether its current old OTA slot overlaps the destination at `0x10000`. The stock
   1 MB OTA slots do not overlap. For the historical repository layout with 2 MB slots, it
   first copies and verifies itself in the non-overlapping old `ota_1`, selects it, and
   restarts. This makes migration independent of which old slot was active.
3. Copies and verifies itself at new `ota_0` (`0x10000`) only while executing from a range
   that does not overlap that destination.
4. Erases OTA selection data, replaces the table at `0x8000` with the embedded
   `migration_files/partition_table/partition-table.bin`, and restarts.
5. Under the new table, starts Wi-Fi with the existing NVS settings (or the component's
   fallback access point) and serves a minimal final-firmware uploader at `/`.

NVS and PHY offsets do not move, so Wi-Fi configuration is retained. The updater never
erases or writes its currently executing slot. It verifies both relocations before changing
the table, and changes the table last.

## 1. Validate the supplied table

The checked-in binary must be generated from `partitions_new.csv` using the same ESP-IDF
toolchain as the project. Regenerate it whenever the CSV changes:

```bash
cd ESP32/Digifiz
python "$IDF_PATH/components/partition_table/gen_esp32part.py" \
  partitions_new.csv migration_files/partition_table/partition-table.bin
python "$IDF_PATH/components/partition_table/gen_esp32part.py" \
  migration_files/partition_table/partition-table.bin
```

Confirm the decoded result contains `ota_0` at `0x10000`, `ota_1` at `0x400000`, and no
factory partition. The application slots are `0x3f0000` bytes each.

## 2. Build the one-time migration application

Use a clean, separate build directory. `sdkconfig.defaults` selects ESP32-S3, 8 MB flash,
the **old** temporary build layout, and allows the intentional write to the protected
partition-table region.

```bash
cd ESP32/Digifiz/migration_app
rm -rf build sdkconfig
idf.py set-target esp32s3
idf.py build
```

The file to upload is:

```text
ESP32/Digifiz/migration_app/build/DigifizPartitionMigration.bin
```

Do not upload `bootloader.bin`, `partition-table.bin`, or a merged flash image through the
web OTA form. The migration application must fit in the stock old 1 MB app slot.

## 3. Test on a development device

Before touching field devices:

1. Flash a development ESP32-S3 with the exact old production layout and firmware.
2. Test once with old `ota_0` active and once with old `ota_1` active.
3. Upload `DigifizPartitionMigration.bin` from the normal Digifiz **WiFi** tab.
4. Keep power applied through the automatic restarts (there can be two restarts).
5. Reconnect to the device and open `http://<device-address>/`. Verify it says
   **Migration complete**.
6. Upload the expanded normal application and verify another restart returns to Digifiz.
7. Read back/decode the table or inspect boot logs and confirm the new offsets.

## 4. Build the normal expanded application

The normal application must now be built against `partitions_new.csv`. A prepared
`sdkconfig_new` is included and points at that file:

```bash
cd ESP32/Digifiz
cp sdkconfig_new sdkconfig
idf.py fullclean
idf.py build
```

Check that the build reports an 8 MB flash and the custom `partitions_new.csv` table. The
final web-upload file is `build/Digifiz.bin`; it must be smaller than `0x3f0000` bytes.

## 5. Migrate a field device

For each device:

1. Ensure ignition/device power and Wi-Fi will remain stable for the whole operation.
2. Open the existing Digifiz web interface and its **WiFi** tab.
3. In **OTA Update software**, upload the tested
   `DigifizPartitionMigration.bin` exactly as a normal firmware update.
4. Do not remove power. Allow up to two automatic restarts and reconnect to the same Wi-Fi
   address. If saved STA credentials cannot connect, join the fallback AP configured by the
   `nvs_wifi_connect` component and browse to `192.168.4.1`.
5. Open `/` and require the **Migration complete** message. Never upload the final image if
   the page says migration is incomplete.
6. Select the tested `build/Digifiz.bin` on that page and click **Upload firmware**.
7. Wait for the success response and restart, then verify the full Digifiz interface and
   settings.

## Failure handling

- If the migration app reports an error in serial logs before replacing the table, the old
  table is still present; keep the unit powered and diagnose on a development device before
  retrying.
- If Wi-Fi disappears briefly, wait for the restart and reconnect; do not power-cycle during
  relocation.
- If power is lost during the final partition-table erase/write, USB recovery may be the only
  option. There is no fully atomic way to replace that flash sector.
- Keep the migration binary and its matching table binary archived with the release. Do not
  reuse the migration application for future layouts.
