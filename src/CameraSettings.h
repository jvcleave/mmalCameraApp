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
	int enable;       /// Turn colourFX on or off
	int u,v;          /// U and V to use
};

class CameraSettings
{
public:
	CameraSettings();
	int sharpness;             /// -100 to 100
	int contrast;              /// -100 to 100
	int brightness;            ///  0 to 100
	int saturation;            ///  -100 to 100
	int ISO;                   ///  TODO : what range?
	int videoStabilisation;    /// 0 or 1 (false or true)
	int exposureCompensation;  /// -10 to +10 ?
	MMAL_PARAM_EXPOSUREMODE_T	exposureMode;
	MMAL_PARAM_EXPOSUREMETERINGMODE_T exposureMeterMode;
	MMAL_PARAM_AWBMODE_T awbMode;
	MMAL_PARAM_IMAGEFX_T imageEffect;
	MMAL_PARAMETER_IMAGEFX_PARAMETERS_T imageEffectsParameters;
	MMAL_PARAM_COLOURFX_T colourEffects;
	int rotation;              /// 0-359
	bool hflip;                 /// 0 or 1
	bool vflip;                 /// 0 or 1
};
