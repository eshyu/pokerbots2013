#!/bin/bash
g++ -O3 -ggdb -DDEBUG -Wall -Wformat-security -Wfloat-equal -Wpointer-arith -Wcast-qual -Wsign-compare -pedantic -ansi -I../src/ -I../src/include/ ../src/floatfann.c training.cpp -o train -lm 
./train > dump.txt