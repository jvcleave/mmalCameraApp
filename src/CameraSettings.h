#pragma once

#include "ofMain.h"

#include "interface/mmal/mmal.h"
#include "interface/mmal/mmal_logging.h"
#include "interface/mmal/mmal_buffer.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"

struct MMAL_PARAM_COLOURFX_T
{
	int enable;       // Turn colourFX on or off
	int u,v;          // U and V to use
};

class CameraSettings
{
public:
	CameraSettings();
	int sharpness;             // -100 to 100
	int contrast;              // -100 to 100
	int brightness;            //  0 to 100
	int saturation;            //  -100 to 100
	int ISO;                   //  TODO : what range?
	int videoStabilisation;    // 0 or 1 (false or true)
	int exposureCompensation;  // -10 to +10 ?
	MMAL_PARAM_EXPOSUREMODE_T			exposureMode;
	MMAL_PARAM_EXPOSUREMETERINGMODE_T	exposureMeterMode;
	MMAL_PARAM_AWBMODE_T				awbMode;
	MMAL_PARAM_IMAGEFX_T				imageEffect;
	MMAL_PARAMETER_IMAGEFX_PARAMETERS_T imageEffectsParameters;
	MMAL_PARAM_COLOURFX_T				colourEffects;
	int rotation;              // 0-359
	bool hflip;                 // 0 or 1
	bool vflip;                 // 0 or 1
	
	MMAL_COMPONENT_T *camera;
	void setup(MMAL_COMPONENT_T *camera_);
	
	void									set_saturation(int saturation);
	void									set_sharpness(int sharpness);
	void									set_contrast(int contrast);
	void									set_brightness(int brightness);
	void									set_ISO(int ISO);
	void									set_metering_mode(MMAL_PARAM_EXPOSUREMETERINGMODE_T mode);
	void									set_video_stabilisation(int vstabilisation);
	void									set_exposure_compensation(int exp_comp);
	void									set_exposure_mode(MMAL_PARAM_EXPOSUREMODE_T mode);
	void									set_awb_mode(MMAL_PARAM_AWBMODE_T awb_mode);
	void									set_imageFX(MMAL_PARAM_IMAGEFX_T imageFX);
	void									set_colourFX(const MMAL_PARAM_COLOURFX_T *colourFX);
	void									set_rotation(int rotation);
	void									set_flips(bool hflip, bool vflip);
	
	int									get_saturation();
	int									get_sharpness();
	int									get_contrast();
	int									get_brightness();
	int									get_ISO();
	MMAL_PARAM_EXPOSUREMETERINGMODE_T	get_metering_mode();
	int									get_video_stabilisation();
	int									get_exposure_compensation();
	//MMAL_PARAM_THUMBNAIL_CONFIG_T		get_thumbnail_parameters();
	MMAL_PARAM_EXPOSUREMODE_T			get_exposure_mode();
	MMAL_PARAM_AWBMODE_T				get_awb_mode();
	MMAL_PARAM_IMAGEFX_T				get_imageFX();
	MMAL_PARAM_COLOURFX_T				get_colourFX();
};
