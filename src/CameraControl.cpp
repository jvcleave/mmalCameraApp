/*
 *  CameraControl.cpp
 *  openFrameworksLib
 *
 *  Created by jason van cleave on 5/16/13.
 *  Copyright 2013 jasonvancleave.com. All rights reserved.
 *
 */

#include "CameraControl.h"

CameraControl::CameraControl()
{

}

void CameraControl::setup(MMAL_COMPONENT_T *camera, const RASPICAM_CAMERA_PARAMETERS *params)
{
	/*int result;
	
	result  = raspicamcontrol_set_saturation(camera, params->saturation);
	result += raspicamcontrol_set_sharpness(camera, params->sharpness);
	result += raspicamcontrol_set_contrast(camera, params->contrast);
	result += raspicamcontrol_set_brightness(camera, params->brightness);
	//result += raspicamcontrol_set_ISO(camera, params->ISO); TODO Not working for some reason
	result += raspicamcontrol_set_video_stabilisation(camera, params->videoStabilisation);
	result += raspicamcontrol_set_exposure_compensation(camera, params->exposureCompensation);
	result += raspicamcontrol_set_exposure_mode(camera, params->exposureMode);
	result += raspicamcontrol_set_metering_mode(camera, params->exposureMeterMode);
	result += raspicamcontrol_set_awb_mode(camera, params->awbMode);
	result += raspicamcontrol_set_imageFX(camera, params->imageEffect);
	result += raspicamcontrol_set_colourFX(camera, &params->colourEffects);
	//result += raspicamcontrol_set_thumbnail_parameters(camera, &params->thumbnailConfig);  TODO Not working for some reason
	result += raspicamcontrol_set_rotation(camera, params->rotation);
	result += raspicamcontrol_set_flips(camera, params->hflip, params->vflip);
	
	return result;*/
}
						  
