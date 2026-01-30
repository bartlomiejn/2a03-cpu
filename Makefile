SRC_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

OUT_DIR ?= $(SRC_DIR)/output
CXX_DEBUG ?= gdb
BIN ?= nestest

ifeq ($(BIN), nestest)
	ARGS ?= -cebmtl nestest_neslog
else ifeq ($(BIN), nestesti)
	ARGS ?= -ti
else ifeq ($(BIN), pputest)
	ARGS ?= -cebmyl pputest_neslog
else ifeq ($(BIN), cputest)
	ARGS ?= -cebmul cputest_neslog
else ifeq ($(BIN), dk)
	ARGS ?= -cr DonkeyKong.nes
else
	ARGS ?= -r $(BIN)
endif

.PHONY: binary run debug vg check lint loc clean

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
	cd $(OUT_DIR) && $(CXX_DEBUG) $(GDBARGS) --args ./2a03 $(ARGS)

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
