SRC_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
OUT_DIR ?= output
ASM_DIR := tools/acme091
ASM := acme

.PHONY: tools

tools:
	cd $(ASM_DIR)/src && $(MAKE)
	cd $(ASM_DIR)/src && $(MAKE) install