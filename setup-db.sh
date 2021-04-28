#!/bin/bash


sdskv-server-daemon -f serveraddr.txt tcp pmpiDB &
cat serveraddr.txt > input.txt
cat otherargs.txt >> input.txt
