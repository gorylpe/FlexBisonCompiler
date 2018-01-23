#!/bin/bash
make clean
make
./compiler < test > prog
labor4/interpreter-cln prog
