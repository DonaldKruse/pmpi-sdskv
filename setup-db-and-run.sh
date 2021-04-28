#!/bin/bash


sdskv-server-daemon -f serveraddr.txt tcp pmpiDB &
cat serveraddr.txt > input.txt
cat otherargs.txt >> input.txt
LD_PRELOAD=./pmpi-lib.so lrun -N 2 -T 1 ./main-generate $(<input.txt)

./main-get $(<input.txt)
