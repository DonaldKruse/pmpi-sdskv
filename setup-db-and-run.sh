#!/bin/bash


sdskv-server-daemon -f serveraddr.txt tcp pmpiDB &
sleep 3
cat serveraddr.txt > input.txt
cat otherargs.txt >> input.txt
LD_PRELOAD=./pmpi-lib.so lrun -N 2 -T 1 ./main-generate $(<input.txt)
sleep 3
./main-get $(<input.txt)
