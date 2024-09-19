SRC_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
OUT_DIR ?= $(SRC_DIR)/output
CXX_DEBUG ?= gdb

.PHONY: binary run debug lint clean

$(OUT_DIR):
	mkdir -p $@

binary: $(OUT_DIR)
	cd $(OUT_DIR) && cmake \
		-DCMAKE_BUILD_TYPE=Debug \
		-D2A03_INCLUDE_TESTS=TRUE \
		-Wall -Werror \
		$(SRC_DIR)
	$(MAKE) 2a03 -C $(OUT_DIR)

run_nestest: binary
	cd $(OUT_DIR) && ./2a03 -ct

run_pputest: binary
	cd $(OUT_DIR) && ./2a03 -cp

debug: binary
	cd $(OUT_DIR) && $(CXX_DEBUG) 2a03

lint:
	find ./src ./include -name "*.cpp" -o -name "*.h" | xargs clang-format -i

clean:
	rm -rf $(OUT_DIR)
