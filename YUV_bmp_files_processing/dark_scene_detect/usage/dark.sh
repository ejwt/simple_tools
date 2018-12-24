#!/bin/bash
if [ $# -lt 1 ]; then
    echo " usage: $0 APL_detect_threshold"
    exit
fi

./dark_scene_detect.exe -i DAT.yuv -w 128 -h 72 -fps 25 -t $1
./dark_scene_detect.exe -i DAT_001.yuv -w 128 -h 72 -fps 25 -t $1
./dark_scene_detect.exe -i DAT_002.yuv -w 128 -h 72 -fps 25 -t $1
./dark_scene_detect.exe -i DAT_003.yuv -w 128 -h 72 -fps 25 -t $1
./dark_scene_detect.exe -i DAT_004.yuv -w 128 -h 72 -fps 25 -t $1
./dark_scene_detect.exe -i DAT_005.yuv -w 128 -h 72 -fps 25 -t $1
./dark_scene_detect.exe -i DAT_006.yuv -w 128 -h 72 -fps 25 -t $1
./dark_scene_detect.exe -i DAT_007.yuv -w 128 -h 72 -fps 25 -t $1
./dark_scene_detect.exe -i DAT_008.yuv -w 128 -h 72 -fps 25 -t $1
./dark_scene_detect.exe -i DAT_009.yuv -w 128 -h 72 -fps 25 -t $1
./dark_scene_detect.exe -i DAT_010.yuv -w 128 -h 72 -fps 25 -t $1
