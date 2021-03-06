#pragma once

#include "ofMain.h"




// Standard port setting for the camera component
#define MMAL_CAMERA_CAPTURE_PORT 2



// Stills format information
#define STILLS_FRAME_RATE_NUM 3
#define STILLS_FRAME_RATE_DEN 1

#define OUTPUT_BUFFERS_NUM 3


#include "CameraSettings.h"
#include "Photo.h"

struct PORT_USERDATA
{
	FILE *file_handle;						// File handle to write buffer data to.
	VCOS_SEMAPHORE_T complete_semaphore;	// semaphore which is posted when we reach end of frame (indicates end of capture or fault)
	Photo *photo;							// pointer to our state in case required in callback
};

class ofxRaspicam
{
public:
	ofxRaspicam();
	~ofxRaspicam();
	void setup();
	void takePhoto();
	ofImage lastImage;
private:
	Photo photo;
	void create_camera_component();
	void create_encoder_component();
	MMAL_PORT_T* camera_still_port;
	MMAL_PORT_T* encoder_input_port;
	MMAL_PORT_T* encoder_output_port;
	MMAL_COMPONENT_T* camera;
	MMAL_COMPONENT_T* encoder;
	
	PORT_USERDATA callback_data;
	
};