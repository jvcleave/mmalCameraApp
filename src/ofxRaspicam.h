#pragma once

#include "ofMain.h"
#include "CameraControl.h"
//#include "interface/vcos/vcos.h"





class ofxRaspicam
{
public:
	ofxRaspicam();
	~ofxRaspicam();
	void setup();
	RASPISTILL_STATE state;
	void create_camera_component();
	void create_encoder_component();
	MMAL_PORT_T* camera_preview_port;
	MMAL_PORT_T* camera_video_port;
	MMAL_PORT_T* camera_still_port;
	MMAL_PORT_T* encoder_input_port;
	MMAL_PORT_T* encoder_output_port;
	MMAL_COMPONENT_T *camera;
	MMAL_COMPONENT_T *encoder;
	
	CameraControl cameraController;
	PORT_USERDATA callback_data;
};