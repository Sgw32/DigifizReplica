cmake_minimum_required(VERSION 3.16)

list(APPEND EXTRA_COMPONENT_DIRS "../components/nvs_wifi_connect")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(DigifizPartitionMigration)
