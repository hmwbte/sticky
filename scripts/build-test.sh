#!/bin/bash

mkdir ./build
mkdir ./build/test

g++ -std=c++20 -o ./build/test/test ./test/test.cpp ./src/sticky.cpp