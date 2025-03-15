# TDT4230 - Graphics and Visualization

This is the base code for the NTNU subject TDT4230 assignments. It may also be used for the final project, which we will showcase [here](https://www.idi.ntnu.no/grupper/vis/teaching/)

## What do i do?

	git clone --recursive https://github.com/bartvbl/TDT4230-Assignment-1.git

Should you forget the `--recursive` bit, just run:

	git submodule update --init


### Windows

Install Microsoft Visual Studio Express and CMake.
You may use CMake-gui or the command-line cmake to generate a Visual Studio solution.

### Linux:

Make sure you have a C/C++ compiler such as GCC, CMake, Python and Git.

	make run

which is equivalent to

	git submodule update --init
	cd build
	cmake ..
	make
	./glowbox

#### other nifty tools

If you want to debug a crash:

  make run-debug

If you want to debug a frame with [renderdoc](https://renderdoc.org/):

  make run-renderdoc
