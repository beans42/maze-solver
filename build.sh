#!/bin/sh
g++ -o maze -O3 src/main.cpp imgui/*.cpp tinyfiledialogs/tinyfiledialogs.c -lGL -lGLEW -lglfw