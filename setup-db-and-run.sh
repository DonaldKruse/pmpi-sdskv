#!/bin/bash


#sdskv-server-daemon -f serveraddr.txt tcp pmpiDB &
#sleep 2
#cat serveraddr.txt > input.txt
#cat otherargs.txt >> input.txt
#sleep 1

#LD_PRELOAD=./pmpi-lib.so lrun -N 2 -T 1 ./main-generate $(<input.txt)
#sleep 3
#./main-get $(<input.txt)

#for i in {1,2,4,8,16,32,64}
#do
#
#    # build param file
#    # right now there are only 5 params to add that correspond to threshold values
#    truncate -s 0 pmpi-params.txt
#    for k in {1..5}; do echo "$i" >> pmpi-params.txt; done
#
#    sdskv-server-daemon -f serveraddr.txt tcp pmpiDB &
#    sleep 2
#    cat serveraddr.txt > input.txt
#    cat otherargs.txt >> input.txt
#    filename="main-generate-isend-times-T$i.txt"
#    echo -n " main-generate-isend-times-T$i.txt" >> input.txt
#    sleep 1
#    
#    LD_PRELOAD=./pmpi-lib.so lrun -N 2 -T 1 ./main-generate $(<input.txt)
#    sleep 2
#    ./main-get $(<input.txt)
#    sleep 2
#
#    mv $filename timings
#done
# run without pmpi
#cat serveraddr.txt > input.txt
#cat otherargs.txt >> input.txt
#echo -n " main-generate-isend-times-no-pmpi.txt" >> input.txt
#lrun -N 2 -T 1 ./main-generate $(<input.txt)
#mv main-generate-isend-times-no-pmpi.txt timings


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

#LD_PRELOAD=./pmpi-lib.so lrun -N 4 -T 1 ./pi/mpi_pi_send $(<input.txt)
#sleep 3
#./main-get $(<input.txt)



