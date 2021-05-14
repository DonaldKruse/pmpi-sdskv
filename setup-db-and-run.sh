#!/bin/bash


sdskv-server-daemon -f serveraddr.txt tcp pmpiDB &
sleep 2
cat serveraddr.txt > input.txt
cat otherargs.txt >> input.txt
sleep 1

#LD_PRELOAD=./pmpi-lib.so lrun -N 2 -T 1 ./main-generate $(<input.txt)
#sleep 3
#./main-get $(<input.txt)

#LD_PRELOAD=./pmpi-lib.so lrun -N 2 -T 1 ./iter-send-recv $(<input.txt)
#sleep 3
#./main-get $(<input.txt)

#echo "running ping-pong"
#LD_PRELOAD=./pmpi-lib.so lrun -N 2 -T 1 ./ping-pong $(<input.txt)
#sleep 3
#echo "running main-get"
#./main-get $(<input.txt)

#LD_PRELOAD=./pmpi-lib.so lrun -N 4 -T 1 ./pi/mpi_pi_reduce $(<input.txt)
#sleep 3
#./main-get $(<input.txt)

LD_PRELOAD=./pmpi-lib.so lrun -N 4 -T 1 ./pi/mpi_pi_send $(<input.txt)
sleep 3
./main-get $(<input.txt)
