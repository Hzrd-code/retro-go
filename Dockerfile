FROM espressif/idf:release-v4.4

WORKDIR /app

ADD . /app

# Apply patches
RUN cd /opt/esp/idf && \
    patch --ignore-whitespace -p1 -i "/app/tools/patches/panic-hook (esp-idf 4).diff" && \
    patch --ignore-whitespace -p1 -i "/app/tools/patches/sdcard-fix (esp-idf 4).diff"

# Indstil bash shell
SHELL ["/bin/bash", "-c"]

# Byg projektet og sammensmelt til en ren AIO-fil
RUN . /opt/esp/idf/export.sh && \
    python rg_tool.py --target=t-deck-plus build-img all && \
    mkdir -p /app/dist && \
    BOOTLOADER=$(find build -name "bootloader.bin" | head -n 1) && \
    PARTITION=$(find build -name "partition-table.bin" | head -n 1) && \
    LAUNCHER=$(find build -name "launcher.bin" | head -n 1) && \
    esptool.py --chip esp32s3 merge_bin -o /app/dist/retro-go-t-deck-plus-AIO.bin \
      0x0 $BOOTLOADER \
      0x8000 $PARTITION \
      0x10000 $LAUNCHER
