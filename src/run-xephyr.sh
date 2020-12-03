#!/bin/sh

xDisplay=":80"

Xephyr $xDisplay -ac -screen 1600x900 &
sleep 1

export DISPLAY=$xDisplay

xterm &

echo "Setup done, now running :"
echo $@

$@