# stm32-st7789-gfx - host tests, equivalence checks and tooling
#
#   make test      run everything below
#   make golden    regenerate tests/golden/pages.golden (do this on purpose)
#   make shots     render every demo and self-test page to PNG (needs Pillow)
#   make clean
#
# Override the compiler with CC=..., the interpreter with PYTHON=...

ifeq ($(origin CC),default)
CC := gcc
endif
CLANG   ?= clang
ARMGCC  ?= arm-none-eabi-gcc
ARMSIZE ?= arm-none-eabi-size
PYTHON  ?= $(shell command -v python3 >/dev/null 2>&1 && echo python3 || echo python)

ifneq (,$(or $(findstring Windows,$(OS)),$(findstring mingw,$(shell $(CC) -dumpmachine 2>/dev/null))))
EXE := .exe
# Rtools' make empties TEMP/TMP, and gcc then cannot create temporary files.
$(shell mkdir -p build)
WINTMP := $(shell cygpath -m '$(CURDIR)/build' 2>/dev/null || echo '$(CURDIR)/build')
export TMP  := $(WINTMP)
export TEMP := $(WINTMP)
else
EXE :=
endif

B        := build
INC      := -Iinclude -Itests/host
LIB_SRC  := $(sort $(wildcard src/*.c))
TESTDIR  := tests/host
STRICT   := -std=c99 -Wall -Wextra -pedantic -Werror
CFLAGS   ?= -O1
LDLIBS   := -lm

MULTI_SRC  := $(LIB_SRC) $(TESTDIR)/sim.c
SINGLE_SRC := $(TESTDIR)/single_impl.c $(TESTDIR)/sim.c
SINGLE_INC := -Isingle_header -Itests/host -DUSE_SINGLE_HEADER

.PHONY: all test golden shots clean header header-check host single equiv bench strict clang arm site

all: test

test: header-check host single equiv bench strict clang arm site
	@echo
	@echo "make test: all checks passed"

$(B):
	@mkdir -p $(B)

# ---- single header ----
header:
	$(PYTHON) tools/make_single_header.py

header-check:
	$(PYTHON) tools/make_single_header.py --check

# ---- (a) host tests, multi-file and single-header builds ----
$(B)/test_multi$(EXE): $(TESTDIR)/test_gfx.c $(MULTI_SRC) | $(B)
	$(CC) $(CFLAGS) $(INC) $^ -o $@ $(LDLIBS)

$(B)/test_single$(EXE): $(TESTDIR)/test_gfx.c $(SINGLE_SRC) single_header/lcd_gfx.h | $(B)
	$(CC) $(CFLAGS) $(SINGLE_INC) $(TESTDIR)/test_gfx.c $(SINGLE_SRC) -o $@ $(LDLIBS)

host: $(B)/test_multi$(EXE)
	@echo "== host tests (multi-file)"
	./$(B)/test_multi$(EXE) | tee $(B)/host.log
	@grep -q "^part A: 56 checks" $(B)/host.log || (echo "expected 56 checks in part A"; exit 1)
	@grep -q " 0 failures" $(B)/host.log || (echo "failures found"; exit 1)

single: $(B)/test_single$(EXE)
	@echo "== host tests (single header, two translation units)"
	./$(B)/test_single$(EXE) | tee $(B)/single.log
	@grep -q " 0 failures" $(B)/single.log || (echo "failures found"; exit 1)

# ---- (b) pixel equivalence: per-page FNV-1a hashes against the golden file ----
$(B)/pages_multi$(EXE): $(TESTDIR)/pages.c $(MULTI_SRC) | $(B)
	$(CC) $(CFLAGS) $(INC) $^ -o $@ $(LDLIBS)

$(B)/pages_single$(EXE): $(TESTDIR)/pages.c $(SINGLE_SRC) single_header/lcd_gfx.h | $(B)
	$(CC) $(CFLAGS) $(SINGLE_INC) $(TESTDIR)/pages.c $(SINGLE_SRC) -o $@ $(LDLIBS)

equiv: $(B)/pages_multi$(EXE) $(B)/pages_single$(EXE)
	@echo "== pixel equivalence (multi-file vs single header vs golden)"
	./$(B)/pages_multi$(EXE) hash > $(B)/pages_multi.txt
	./$(B)/pages_single$(EXE) hash > $(B)/pages_single.txt
	diff -u --strip-trailing-cr $(B)/pages_multi.txt $(B)/pages_single.txt
	diff -u --strip-trailing-cr tests/golden/pages.golden $(B)/pages_multi.txt
	@echo "equivalence OK: $$(wc -l < $(B)/pages_multi.txt) pages identical"

golden: $(B)/pages_multi$(EXE)
	./$(B)/pages_multi$(EXE) hash | tr -d '' > tests/golden/pages.golden
	@echo "wrote tests/golden/pages.golden"

# ---- (c) benchmark ----
$(B)/bench$(EXE): $(TESTDIR)/bench.c $(MULTI_SRC) | $(B)
	$(CC) $(CFLAGS) $(INC) $^ -o $@ $(LDLIBS)

bench: $(B)/bench$(EXE)
	@echo "== bus-write benchmark"
	./$(B)/bench$(EXE)

# ---- (d) strict C99 with gcc and clang ----
strict: | $(B)
	@echo "== strict C99 (gcc): -Wall -Wextra -pedantic -Werror"
	@for f in $(LIB_SRC) $(TESTDIR)/sim.c $(TESTDIR)/single_impl.c; do \
		$(CC) $(STRICT) -Iinclude -Isingle_header -c $$f -o $(B)/strict.o || exit 1; done
	@for f in $(TESTDIR)/test_gfx.c $(TESTDIR)/pages.c $(TESTDIR)/bench.c; do \
		$(CC) $(STRICT) $(INC) -c $$f -o $(B)/strict.o || exit 1; \
		$(CC) $(STRICT) $(SINGLE_INC) -c $$f -o $(B)/strict.o || exit 1; done
	@echo "strict gcc OK"

clang: | $(B)
	@if command -v $(CLANG) >/dev/null 2>&1; then \
		echo "== strict C99 (clang)"; \
		for f in $(LIB_SRC) $(TESTDIR)/sim.c $(TESTDIR)/single_impl.c; do \
			$(CLANG) $(STRICT) -Iinclude -Isingle_header -c $$f -o $(B)/strict.o || exit 1; done; \
		for f in $(TESTDIR)/test_gfx.c $(TESTDIR)/pages.c $(TESTDIR)/bench.c; do \
			$(CLANG) $(STRICT) $(INC) -c $$f -o $(B)/strict.o || exit 1; done; \
		$(CLANG) $(CFLAGS) $(INC) $(TESTDIR)/test_gfx.c $(MULTI_SRC) -o $(B)/test_clang$(EXE) $(LDLIBS) && ./$(B)/test_clang$(EXE) | tail -1; \
		echo "strict clang OK"; \
	else echo "== clang not found: skipped (CI runs it)"; fi

# ---- (e) cross compile for Cortex-M7 ----
arm: | $(B)
	@if command -v $(ARMGCC) >/dev/null 2>&1; then \
		echo "== cross compile: $(ARMGCC) -mcpu=cortex-m7"; \
		rm -f $(B)/arm_*.o; \
		for f in $(LIB_SRC); do \
			$(ARMGCC) -std=c99 -Wall -Wextra -pedantic -Werror -Os -mcpu=cortex-m7 -mthumb -ffunction-sections -fdata-sections \
				-Iinclude -c $$f -o $(B)/arm_$$(basename $$f .c).o || exit 1; done; \
		$(ARMSIZE) $(B)/arm_*.o; \
		echo "arm OK"; \
	else echo "== $(ARMGCC) not found: skipped (CI runs it)"; fi

# ---- web checks ----
site:
	@echo "== site checks"
	$(PYTHON) tools/check_site.py

# ---- screenshots ----
shots: $(B)/pages_multi$(EXE)
	@mkdir -p $(B)/ppm
	./$(B)/pages_multi$(EXE) render $(B)/ppm
	$(PYTHON) tools/ppm2png.py $(B)/ppm assets/pages

clean:
	rm -rf $(B)
