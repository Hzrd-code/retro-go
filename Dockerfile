FROM espressif/idf:release-v4.4

WORKDIR /app

ADD . /app

# Apply patches
RUN cd /opt/esp/idf && \
    patch --ignore-whitespace -p1 -i "/app/tools/patches/panic-hook (esp-idf 4).diff" && \
    patch --ignore-whitespace -p1 -i "/app/tools/patches/sdcard-fix (esp-idf 4).diff"

# Indstil bash shell
SHELL ["/bin/bash", "-c"]

# Tving AIO-konfiguration i sdkconfig og kør det rene build
RUN echo "CONFIG_RETRO_GO_BUILD_TYPE_ALL_IN_ONE=y" >> sdkconfig.defaults && \
    . /opt/esp/idf/export.sh && \
    python rg_tool.py --target=t-deck-plus build-img all
