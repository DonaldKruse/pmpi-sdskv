#!/bin/bash

#/g/g15/kruse11/Mochi/mochi-sdskv-fork/build/bin/sdskv-server-daemon -f serveraddr.txt tcp putGetTest &
/g/g15/kruse11/Mochi/mochi-sdskv-fork/build/bin/sdskv-server-daemon -f serveraddr.txt tcp putGetTest &
#sdskv-server-daemon -f serveraddr.txt tcp putGetTest &
cat serveraddr.txt > input.txt
cat otherargs.txt >> input.txt

