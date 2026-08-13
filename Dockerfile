FROM espressif/idf:release-v4.4

WORKDIR /app

ADD . /app

# Apply patches
RUN cd /opt/esp/idf && \
    patch --ignore-whitespace -p1 -i "/app/tools/patches/panic-hook (esp-idf 4).diff" && \
    patch --ignore-whitespace -p1 -i "/app/tools/patches/sdcard-fix (esp-idf 4).diff"

# Indstil bash shell
SHELL ["/bin/bash", "-c"]

# 1. Byg projektet helt normalt
# 2. Brug esptool til manuelt at sammensmelte alt til én stor AIO-fil
RUN . /opt/esp/idf/export.sh && \
    python rg_tool.py --target=t-deck-plus build-img all && \
    mkdir -p /app/dist && \
    cd build && \
    esptool.py --chip esp32s3 merge_bin -o /app/dist/retro-go-t-deck-plus-AIO.bin \
      0x1000 bootloader.bin \
      0x8000 partition-table.bin \
      0x10000 launcher.bin
