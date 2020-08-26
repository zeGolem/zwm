#!/bin/sh

xDisplay=":80"

Xephyr $xDisplay -ac -screen 1280x720 &
sleep 1

export DISPLAY=$xDisplay

xclock &
konsole &