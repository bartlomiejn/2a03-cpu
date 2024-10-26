SRC_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

OUT_DIR ?= $(SRC_DIR)/output
CXX_DEBUG ?= gdb
BIN ?= nestest

ifeq ($(BIN), nestest)
	ARGS = -ct 
else ifeq ($(BIN), pputest)
	ARGS = -cp
else ifeq ($(BIN), cputest)
	ARGS = -cu
else ifeq ($(BIN), dk)
	ARGS = -cr DonkeyKong.nes
else
	ARGS = -cr $(BIN)
endif

.PHONY: binary run debug lint clean

$(OUT_DIR):
	mkdir -p $@

binary: $(OUT_DIR)
	cd $(OUT_DIR) && cmake \
		-DCMAKE_BUILD_TYPE=Debug \
		-Wall -Werror \
		$(SRC_DIR)
	$(MAKE) 2a03 -C $(OUT_DIR)

run: binary
	cd $(OUT_DIR) && ./2a03 $(ARGS)

debug: binary
	cd $(OUT_DIR) && $(CXX_DEBUG) --args ./2a03 $(ARGS)

vg: binary
	cd $(OUT_DIR) && valgrind --tool=memcheck --leak-check=full -s \
		--track-origins=yes --log-file=vg.$(BIN).log \
		--suppressions=../vgsuppress ./2a03 $(ARGS)

check:
	$(MAKE) cppcheck -C $(OUT_DIR)
	find ./src -name "*.cpp" -o -name "*.h" | xargs clang-format -i

loc:
	find ./src -type f \( -name \*.cpp -o -name \*.h \) -exec wc -l {} +

clean:
	rm -rf $(OUT_DIR)
