#!/bin/sh
#clang-format -i src/*.c sys/share/*.c win/*/*.c
clang-format -i include/*.h win/curses/*.h

#mv .clang-format .clang-format-bak
#cp .clang-format-header .clang-format
#clang-format -i include/*.h win/curses/*.h
#mv .clang-format-bak .clang-format

