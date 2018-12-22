#!/bin/bash
if [ $# -lt 1 ]; then
    echo " usage: $0 APL_detect_threshold"
    exit
fi

./dark_scene_detect.exe -i dark-rB_80x48p25.yuv    -w   80 -h  48 -fps 25 -t $1
./dark_scene_detect.exe -i dark-rB_160x96p25.yuv   -w  160 -h  96 -fps 25 -t $1
./dark_scene_detect.exe -i dark-rB_320x184p25.yuv  -w  320 -h 184 -fps 25 -t $1
./dark_scene_detect.exe -i dark-rB_640x360p25.yuv  -w  640 -h 360 -fps 25 -t $1
./dark_scene_detect.exe -i dark-rB_1280x720p25.yuv -w 1280 -h 720 -fps 25 -t $1
./dark_scene_detect.exe -i dark-rB_1280x720p50.yuv -w 1280 -h 720 -fps 50 -t $1
