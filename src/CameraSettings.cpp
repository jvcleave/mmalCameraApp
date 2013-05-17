/*
 *  CameraSettings.cpp
 *  openFrameworksLib
 *
 *  Created by jason van cleave on 5/17/13.
 *  Copyright 2013 jasonvancleave.com. All rights reserved.
 *
 */

#include "CameraSettings.h"

CameraSettings::CameraSettings()
{
	sharpness = 0;
	contrast = 0;
	brightness = 50;
	saturation = 0;
	ISO = 400;
	videoStabilisation = 0;
	exposureCompensation = 0;
	exposureMode = MMAL_PARAM_EXPOSUREMODE_AUTO;
	exposureMeterMode = MMAL_PARAM_EXPOSUREMETERINGMODE_AVERAGE;
	awbMode = MMAL_PARAM_AWBMODE_AUTO;
	imageEffect = MMAL_PARAM_IMAGEFX_NONE;
	colourEffects.enable = 0;
	colourEffects.u = 128;
	colourEffects.v = 128;
	rotation = 0;
	hflip = false;
	vflip = false;
}
