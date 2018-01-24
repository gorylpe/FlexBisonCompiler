#!/bin/bash
make clean
make
./compiler < test > prog -v
labor4/interpreter-cln prog
