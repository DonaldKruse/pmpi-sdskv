#!/bin/bash


sdskv-server-daemon -f serveraddr.txt tcp pmpiDB &
cat serveraddr.txt > input.txt
cat otherargs.txt >> input.txt
LD_PRELOAD=./bin/pmpi-lib.so lrun -N 2 -T 1 ./bin/main-generate $(<input.txt)

./bin/main-get $(<input.txt)
