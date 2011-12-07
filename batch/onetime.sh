#!/bin/sh

PROCESS=("shift-blur.out" "stereoDepthEstimation.out" "DepthFromDefocus.out" "CodedAperturePair.out")

dir="exp"

leftImage="${dir}/leftImage-01.png"
rightImage="${dir}/rightImage-01.png"

apertureLeft="aperture/Zhou0002.png"
apertureRight="aperture/Zhou0002.png"

paramLeft=(-0.8 19.5716)
paramRight=(-0.8 18.3733)

tmpImages="tmp"
outputImages="estimated.png"



./${PROCESS[0]} ${leftImage} ${rightImage} ${apertureLeft} ${apertureRight} ${paramLeft[0]} ${paramLeft[1]} ${paramRight[0]} ${paramRight[1]} ${tmpImages} ${outputImages}
