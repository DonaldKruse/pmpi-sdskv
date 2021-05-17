#!/bin/bash


#sdskv-server-daemon -f serveraddr.txt tcp pmpiDB &
#sleep 2
#cat serveraddr.txt > input.txt
#cat otherargs.txt >> input.txt
#echo -n " ouput.txt" >> input.txt
#sleep 1

#LD_PRELOAD=./pmpi-lib.so lrun -N 2 -T 1 ./main-generate $(<input.txt)
#sleep 3
#./main-get $(<input.txt)

#LD_PRELOAD=./pmpi-lib.so lrun -N 2 -T 1 ./iter-send-recv $(<input.txt)
#sleep 3
#./main-get $(<input.txt)

#LD_PRELOAD=./pmpi-lib.so lrun -N 2 -T 1 ./ping-pong $(<input.txt)
#sleep 3
#./main-get $(<input.txt)

#LD_PRELOAD=./pmpi-lib.so lrun -N 4 -T 1 ./pi/mpi_pi_reduce $(<input.txt)
#sleep 3
#./main-get $(<input.txt)

#LD_PRELOAD=./pmpi-lib.so lrun -N 4 -T 1 ./pi/mpi_pi_send $(<input.txt)
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
#    filename="mpi-pi-reduce-times-T$i.txt"
#    echo -n " mpi-pi-reduce-times-T$i.txt" >> input.txt
#    sleep 1
#    
#    LD_PRELOAD=./pmpi-lib.so lrun -N 4 -T 1 ./pi/mpi_pi_reduce $(<input.txt) > $filename
#    sleep 2
#    ./main-get $(<input.txt)
#    sleep 2
#    mv $filename timings/mpi_pi_reduce
#
#done
# run without pmpi
#cat serveraddr.txt > input.txt
#cat otherargs.txt >> input.txt
#echo -n " mpi-pi-reduce-times-no-pmpi.txt" >> input.txt
#lrun -N 4 -T 1 ./pi/mpi_pi_reduce $(<input.txt) > mpi-pi-reduce-times-no-pmpi.txt
#mv mpi-pi-reduce-times-no-pmpi.txt timings/mpi_pi_reduce



for i in {1,2,4,8,16,32,64}
do

    # build param file
    # right now there are only 5 params to add that correspond to threshold values
    truncate -s 0 pmpi-params.txt
    for k in {1..5}; do echo "$i" >> pmpi-params.txt; done

    sdskv-server-daemon -f serveraddr.txt tcp pmpiDB &
    sleep 2
    cat serveraddr.txt > input.txt
    cat otherargs.txt >> input.txt
    filename="mpi-pi-send-times-T$i.txt"
    echo -n " mpi-pi-send-times-T$i.txt" >> input.txt
    sleep 1
    
    LD_PRELOAD=./pmpi-lib.so lrun -N 4 -T 1 ./pi/mpi_pi_send $(<input.txt) > $filename
    sleep 2
    ./main-get $(<input.txt)
    sleep 2
    mv $filename timings/mpi_pi_send

done
 run without pmpi
cat serveraddr.txt > input.txt
cat otherargs.txt >> input.txt
echo -n " mpi-pi-send-times-no-pmpi.txt" >> input.txt
lrun -N 4 -T 1 ./pi/mpi_pi_send $(<input.txt) > mpi-pi-send-times-no-pmpi.txt
mv mpi-pi-send-times-no-pmpi.txt timings/mpi_pi_send


