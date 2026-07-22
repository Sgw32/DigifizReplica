idf_component_register(
    SRCS "migration_main.c"
    INCLUDE_DIRS "."
    REQUIRES app_update esp_http_server esp_wifi nvs_flash nvs_wifi_connect spi_flash
    EMBED_FILES
        "../../migration_files/partition_table/partition-table.bin"
        "../migration.html"
)
