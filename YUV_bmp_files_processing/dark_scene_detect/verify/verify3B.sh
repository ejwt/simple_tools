#!/bin/bash
if [ $# -lt 1 ]; then
    echo " usage: $0 APL_detect_threshold"
    exit
fi

./dark_scene_detect.exe -i foreman_part_qcif.yuv     -w  176 -h 144 -fps 25 -t $1 -f yuv420p
./dark_scene_detect.exe -i foreman_part_qcif_422.yuv -w  176 -h 144 -fps 25 -t $1 -f yuv422p
./dark_scene_detect.exe -i foreman_part_qcif_444.yuv -w  176 -h 144 -fps 25 -t $1 -f yuv444p

./dark_scene_detect.exe -i tulips_yuv420_prog_planar_qcif.yuv -w  176 -h 144 -fps 25 -t $1 -f yuv420p
./dark_scene_detect.exe -i tulips_yuv422_prog_planar_qcif.yuv -w  176 -h 144 -fps 25 -t $1 -f yuv422p
./dark_scene_detect.exe -i tulips_yuv444_prog_planar_qcif.yuv -w  176 -h 144 -fps 25 -t $1 -f yuv444p
