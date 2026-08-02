FROM espressif/idf:release-v4.4

WORKDIR /app

ADD . /app

# Apply patches
RUN cd /opt/esp/idf && \
	patch --ignore-whitespace -p1 -i "/app/tools/patches/panic-hook (esp-idf 4).diff" && \
	patch --ignore-whitespace -p1 -i "/app/tools/patches/sdcard-fix (esp-idf 4).diff"

# Kør reconfigure for at bage alle cores ind i launcheren (All-In-One)
SHELL ["/bin/bash", "-c"]
RUN . /opt/esp/idf/export.sh && \
    python rg_tool.py --target=t-deck-plus build-img all CONFIG_RETRO_GO_BUILD_TYPE_ALL_IN_ONE=y
