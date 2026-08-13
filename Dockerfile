FROM espressif/idf:release-v4.4

WORKDIR /app

ADD . /app

# Apply patches
RUN cd /opt/esp/idf && \
    patch --ignore-whitespace -p1 -i "/app/tools/patches/panic-hook (esp-idf 4).diff" && \
    patch --ignore-whitespace -p1 -i "/app/tools/patches/sdcard-fix (esp-idf 4).diff"

# Indstil bash shell
SHELL ["/bin/bash", "-c"]

# Sæt build type til AIO og byg den samlede fil
RUN . /opt/esp/idf/export.sh && \
    export RG_BUILD_TYPE=all-in-one && \
    python rg_tool.py --target=t-deck-plus build-img all && \
    mkdir -p /app/dist && \
    (cp releases/*.bin /app/dist/ 2>/dev/null || true) && \
    (cp build/*.bin /app/dist/ 2>/dev/null || true) && \
    (cp build/t-deck-plus/*.bin /app/dist/ 2>/dev/null || true)
