#!/bin/bash

# TODO: Dockerise?

set -e

source ~/.bashrc
source ~/esp/v5.5.1/esp-idf/export.sh

hexfw=$(printf "0x%08X" "$1")

idf.py build -DDATE_CODE="$(date -u +%Y%m%d)" -DSW_VERSION="0.0.$1" -DFW_VERSION=$hexfw

python3 tools/esp_delta_ota_patch_gen.py create_patch --chip esp32c6 --base_binary build/last.bin --new_binary build/ZigbeeMotion-espidf.bin --patch_file_name build/firmware.bin
python3 tools/image_builder_tool.py --create build/firmware.ota -v $1 -m 0x1001 -i 0x1011 -t 0x0000 -f build/firmware.bin
