#pragma once

#include "ofMain.h"




#define MAX_USER_EXIF_TAGS      32
// Standard port setting for the camera component
#define MMAL_CAMERA_PREVIEW_PORT 0
#define MMAL_CAMERA_VIDEO_PORT 1
#define MMAL_CAMERA_CAPTURE_PORT 2

/// Layer that preview window should be displayed on
#define PREVIEW_LAYER      2
#define PREVIEW_FRAME_RATE_NUM 30
#define PREVIEW_FRAME_RATE_DEN 1


// Stills format information
#define STILLS_FRAME_RATE_NUM 3
#define STILLS_FRAME_RATE_DEN 1


/// Video render needs at least 2 buffers.
#define VIDEO_OUTPUT_BUFFERS_NUM 3












#include "CameraSettings.h"
#include "Photo.h"

struct PORT_USERDATA
{
	FILE *file_handle;                   /// File handle to write buffer data to.
	VCOS_SEMAPHORE_T complete_semaphore; /// semaphore which is posted when we reach end of frame (indicates end of capture or fault)
	Photo *pstate;            /// pointer to our state in case required in callback
};

class ofxRaspicam
{
public:
	ofxRaspicam();
	~ofxRaspicam();
	void setup();
	Photo state;
	void create_camera_component();
	void create_encoder_component();
	MMAL_PORT_T* camera_preview_port;
	MMAL_PORT_T* camera_video_port;
	MMAL_PORT_T* camera_still_port;
	MMAL_PORT_T* encoder_input_port;
	MMAL_PORT_T* encoder_output_port;
	MMAL_COMPONENT_T *camera;
	MMAL_COMPONENT_T *encoder;
	
	PORT_USERDATA callback_data;
};