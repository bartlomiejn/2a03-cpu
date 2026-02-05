SRC_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

OUT_DIR ?= $(SRC_DIR)/output
CXX_DEBUG ?= gdb
BIN ?= nestest
BLD_TYPE ?= Debug
GPROF2DOT_ARGS := -n 0.25 -e 0.05 -f callgrind --color-nodes-by-selftime

ifeq ($(BIN), nestest)
	ARGS ?= -cebmtl nestest_neslog
else ifeq ($(BIN), nestesti)
	BLD_TYPE := Release
	ARGS ?= -ti
else ifeq ($(BIN), nestesti_debug)
	ARGS ?= -cebmtil nestesti_neslog
else ifeq ($(BIN), pputest)
	ARGS ?= -cebmyl pputest_neslog
else ifeq ($(BIN), cputest)
	ARGS ?= -cebmul cputest_neslog
else ifeq ($(BIN), dk)
	BLD_TYPE := Release
	ARGS ?= -r DonkeyKong.nes
else ifeq ($(BIN), dk_debug)
	ARGS ?= -cebmr DonkeyKong.nes -l dk_neslog
else ifeq ($(BIN), dk_cg)
	EN_LOGGING := -DENABLE_LOGGING=OFF
	ARGS ?= -r DonkeyKong.nes -h 20
else
	ARGS ?= -r $(BIN)
endif

ifeq ($(BLD_TYPE), Debug)
	EN_CALLGRIND := -DENABLE_CALLGRIND=ON
endif

.PHONY: binary run debug vg check lint loc clean

$(OUT_DIR):
	mkdir -p $@

binary: $(OUT_DIR)
	cd $(OUT_DIR) && cmake \
		-DCMAKE_BUILD_TYPE=$(BLD_TYPE) \
		$(EN_LOGGING) \
		$(EN_CALLGRIND) \
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

cg: binary
	cd $(OUT_DIR) && valgrind --tool=callgrind --instr-atstart=no \
		--callgrind-out-file=cg.$(BIN) ./2a03 $(ARGS)
	cd $(OUT_DIR) && gprof2dot $(GPROF2DOT_ARGS) -o cg.$(BIN).dot cg.$(BIN)
	cd $(OUT_DIR) && dot -Tpng -o cg.$(BIN).graph.png cg.$(BIN).dot
	cd $(OUT_DIR) &&  eog cg.$(BIN).graph.png

check:
	$(MAKE) cppcheck -C $(OUT_DIR)
	find ./src -name "*.cpp" -o -name "*.h" | xargs clang-format -i

loc:
	find ./src -type f \( -name \*.cpp -o -name \*.h \) -exec wc -l {} +

clean:
	rm -rf $(OUT_DIR)
