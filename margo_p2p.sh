#!/bin/bash

#BSUB -J margo
#BSUB -o margo.out%J
#BSUB -W 10
#BSUB -nnodes 2
#BSUB -q pdebug 

TOP=/g/g15/kruse11/Mochi/mochi-sdskv-fork/pmpi


module load gcc/7.3.1
source /g/g15/kruse11/Mochi/setupMochiEnv.sh
spacktivate -d .
#spack load -r mochi-margo
# disable MR cache in libfabric; still problematic as of libfabric 1.10.1
export FI_MR_CACHE_MAX_COUNT=0
# use shared recv context in RXM; should improve scalability
export FI_OFI_RXM_USE_SRX=1
jsrun -a 1 -n 2 -r 1 -c ALL_CPUS $TOP/bin/margo-p2p-latency -i 100000 -n "verbs://mlx5_0"
jsrun -a 1 -n 2 -r 1 -c ALL_CPUS $TOP/bin/margo-p2p-bw -x 1048576 -n "verbs://mlx5_0" -c 1 -D 20
