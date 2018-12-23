#!/bin/bash
if [ $# -lt 1 ]; then
    echo " usage: $0 APL_detect_threshold"
    exit
fi

./still_scene_detect.exe -i dark-rB_80x48p25.yuv    -w   80 -h  48 -fps 25 -t $1
./still_scene_detect.exe -i dark-rB_160x96p25.yuv   -w  160 -h  96 -fps 25 -t $1
./still_scene_detect.exe -i dark-rB_320x184p25.yuv  -w  320 -h 184 -fps 25 -t $1

./still_scene_detect.exe -i dark-rB_80x48p25_422.yuv    -w   80 -h  48 -fps 25 -t $1 -f yuv422p
./still_scene_detect.exe -i dark-rB_160x96p25_422.yuv   -w  160 -h  96 -fps 25 -t $1 -f yuv422p
./still_scene_detect.exe -i dark-rB_320x184p25_422.yuv  -w  320 -h 184 -fps 25 -t $1 -f yuv422p
