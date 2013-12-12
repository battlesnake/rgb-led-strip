#!/bin/bash

[ -e colors.o ] && rm colors.o

g++ colors.c pfio.c -mcpu=arm1176jzf-s -O3 -o colors.o && chmod +x colors.o && ./colors.o && clear
