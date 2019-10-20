SRC_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
OUT_DIR ?= $(SRC_DIR)/output
CC_DBG ?= gdb

.PHONY: binary run

$(OUT_DIR):
	mkdir -p $@

binary: $(OUT_DIR)
	cd $(OUT_DIR) && cmake \
		-DCMAKE_BUILD_TYPE=Debug \
		-D2A03_INCLUDE_TESTS=TRUE \
		$(SRC_DIR)
	$(MAKE) 2a03 -C $(OUT_DIR)

run: binary
	$(OUT_DIR)/2a03

debug: binary
	$(CC_DBG) $(OUT_DIR)/2a03

clean:
	rm -rf $(OUT_DIR)
