In order to use pmpi header, use
   
   export LD_PRELOAD=./pmpi-lib.so

. Then use
  
  mpirun -n2 ... main-generate

to actually run the function. Note that these have to be 
put in a bsub script, and the export has to happen on the compute
node. Not sure if this works though...
