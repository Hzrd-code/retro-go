FROM espressif/idf:release-v4.4

WORKDIR /app

ADD . /app

# Apply patches
RUN cd /opt/esp/idf && \
	patch --ignore-whitespace -p1 -i "/app/tools/patches/panic-hook (esp-idf 4).diff" && \
	patch --ignore-whitespace -p1 -i "/app/tools/patches/sdcard-fix (esp-idf 4).diff"

# Byg og kopier alle bin-filer sikkert
SHELL ["/bin/bash", "-c"]
RUN . /opt/esp/idf/export.sh && \
	python rg_tool.py --target=t-deck-plus release && \
	mkdir -p /app/dist && \
	find /app/build -type f -name "*.bin" -exec cp {} /app/dist/ \; 2>/dev/null || true
