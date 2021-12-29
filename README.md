# CppND-System-Monitor

## ncurses
[ncurses](https://www.gnu.org/software/ncurses/) is a library that facilitates text-based graphical output in the terminal. This project relies on ncurses for display output.

## Make
This project uses [Make](https://www.gnu.org/software/make/). The Makefile has four targets:
* `build` compiles the source code and generates an executable
* `format` applies [ClangFormat](https://clang.llvm.org/docs/ClangFormat.html) to style the source code
* `debug` compiles the source code and generates an executable, including debugging symbols
* `clean` deletes the `build/` directory, including all of the build artifacts

## Template

![Starting point](/images/starting_monitor.png)

## Finished project (Docker)

Currently the code works on linux only, future work will include macOS and Windows.

![Linux](/images/monitor_linux.png)

## Improved monitor for Linux (Ubuntu)

![Ubuntu](/images/ubuntu.png)