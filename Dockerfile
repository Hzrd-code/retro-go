FROM espressif/idf:release-v4.4

WORKDIR /app

ADD . /app

# Apply patches
RUN cd /opt/esp/idf && \
    patch --ignore-whitespace -p1 -i "/app/tools/patches/panic-hook (esp-idf 4).diff" && \
    patch --ignore-whitespace -p1 -i "/app/tools/patches/sdcard-fix (esp-idf 4).diff"

# Indstil bash shell
SHELL ["/bin/bash", "-c"]

# Byg AIO-billede til T-Deck Plus og saml bin-filerne
RUN . /opt/esp/idf/export.sh && \
    python rg_tool.py --target=t-deck-plus --config=CONFIG_RETRO_GO_BUILD_TYPE_ALL_IN_ONE=y build-img all && \
    mkdir -p /app/dist && \
    cp releases/*.bin /app/dist/ 2>/dev/null || cp build/t-deck-plus/*.bin /app/dist/ 2>/dev/null || cp build/*.bin /app/dist/
