SRC_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
OUT_DIR ?= $(SRC_DIR)/output
ASM_DIR := $(SRC_DIR)/tools/acme091
ASM ?= acme

.PHONY: build_asm binary run

$(OUT_DIR):
	mkdir -p $@

build_asm:
	cd $(ASM_DIR)/src && $(MAKE)
	cd $(ASM_DIR)/src && $(MAKE) install

binary: $(OUT_DIR)
	cd $(OUT_DIR) && cmake $(SRC_DIR)
	$(MAKE) 2a03 -C $(OUT_DIR)

run: binary
	$(OUT_DIR)/2a03
