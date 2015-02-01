#!/bin/bash

echo "Start to test game"

echo "Loading module"

insmod game.ko

lsmod
echo "\n\n"
echo "chenze" > /proc/zero/oppoent
echo "1,1"  >   /proc/zero/game

cat /proc/zero/oppoent
cat /proc/zero/game

echo "2,2" > /proc/zero/game

cat /proc/zero/oppoent
cat /proc/zero/game


rmmod game
tail /var/log/messages 
