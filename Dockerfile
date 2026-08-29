FROM espressif/idf:release-v4.4

WORKDIR /app

ADD . /app

# Apply patches
RUN cd /opt/esp/idf && \
	patch --ignore-whitespace -p1 -i "/app/tools/patches/panic-hook (esp-idf 4).diff" && \
	patch --ignore-whitespace -p1 -i "/app/tools/patches/sdcard-fix (esp-idf 4).diff"

# Use bash so we can use multi-line shell commands
SHELL ["/bin/bash", "-c"]

# Build and produce a single flat .bin in /app/dist
RUN . /opt/esp/idf/export.sh && \
	python rg_tool.py --target=t-deck-plus release && \
	mkdir -p /app/dist && \
	# try to find the produced image (case variations), pick newest if multiple
	IMG="$(ls -1t Retro-Go_*_t-deck-plus.img 2>/dev/null || ls -1t retro-go_*_t-deck-plus.img 2>/dev/null || true)" && \
	if [ -n "$IMG" ]; then \
	  cp "$IMG" /app/dist/retro-go_t-deck-plus.bin; \
	else \
	  echo "ERROR: no image found after packaging. Listing /app:"; ls -la /app; exit 1; \
	fi

# keep final image small-ish by not installing extra stuff
