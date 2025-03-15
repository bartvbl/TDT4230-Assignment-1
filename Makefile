SOURCES := $(shell find src/ -type f | sort | grep -E '\.(h|c)(pp)?$$')
MAKE_OPTS := -j`nproc||echo 4`
GDB_OPTS := -ex "set style enabled on"

.PHONY: help
help:
	@echo -e "Try one of these make targets:\n"
	@grep "^\.PHONY: " Makefile | cut -d" " -f2- | tr -s " " | sed -e "s/ /\n/g" | grep -v "^_" | sed -e "s/^/make /"

.PHONY: run run-with-music run-debug run-renderdoc
run: build
	cd build && ./glowbox
run-with-music: build
	cd build && ./glowbox --enable-music
run-debug: build-debug | has+gdb
	cd build-debug && gdb -batch $(GDB_OPTS) -ex "run" -ex "backtrace" ./glowbox
run-renderdoc: build | has+qrenderdoc has+renderdoccmd
	@echo -e "\nPress F12 to make a frame capture\n"
	@cd build; \
	CAPTURE_DIR="$$(mktemp -d)"; \
	renderdoccmd capture --capture-file "$$CAPTURE_DIR/capture.rdc" --wait-for-exit ./glowbox; \
	test -n "$$(ls "$$CAPTURE_DIR/")" && qrenderdoc "$$CAPTURE_DIR"/capture*.rdc; \
	rm -rf "$$CAPTURE_DIR/" ||:

.PHONY: build
build: build/glowbox
build/glowbox: ${SOURCES} | build/ local+build/lib/ local+build/CMakeFiles/ _submodules has+cmake has+make
	cd build && cmake ..
	mv -f build/compile_commands.json .
	make -C build $(MAKE_OPTS)

.PHONY: build-debug
build-debug: build-debug/glowbox
build-debug/glowbox: ${SOURCES} | build-debug/ local+build-debug/lib/ local+build-debug/CMakeFiles/ _submodules has+cmake has+make
	cd build-debug && cmake -DCMAKE_BUILD_TYPE=Debug ..
	make -C build-debug $(MAKE_OPTS)

.PHONY: _submodules
_submodules: | has+git
	@git submodule update --init

.PHONY: clean
clean:
	@git submodule deinit --all --force
	@test -d build-debug && rm -rfv build-debug/ || true
	@rm -rfv build/ compile_commands.json

# === helpers: ===

# make folders, use as order-only prerequisite
%/:
	mkdir -p $*

# make folders, symlinked to local fs if needed, use as order-only prerequisite
local+%/:
	@if ( df . && df --local .)>&/dev/null; then \
	    if test ! -e "$*"; then \
			test -L "$*" && rm "$*"; \
			ln -s "$$(mktemp -d)/tdt4230-glowbox/$*" "$*"; \
			mkdir -p "$$(readlink "$*")"; \
		fi; \
	else \
		mkdir -p "$*"; \
	fi

# check if program is available in PATH, use as order-only prerequisite
has+%:
	@command -v $* >/dev/null || ( \
		echo "ERROR: Command '$*' not found! Make sure it is installed and available in PATH"; \
		false; \
	) >&2
