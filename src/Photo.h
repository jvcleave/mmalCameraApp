#pragma once

#include "ofMain.h"
#include "interface/mmal/mmal.h"
#include "interface/mmal/mmal_logging.h"
#include "interface/mmal/mmal_buffer.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"
#include "CameraSettings.h"

#define MAX_USER_EXIF_TAGS      32
#define MAX_EXIF_PAYLOAD_LENGTH 128

class Photo
{
public:
	Photo();
	int					timeout;									// Time taken before frame is grabbed and app then shuts down. Units are milliseconds
	int					width;                          
	int					height;                         
	int					quality;									// JPEG quality setting (1-100)
	int					wantRAW;									// Flag for whether the JPEG metadata also contains the RAW bayer image
	char*				filename;									// filename of output file
	MMAL_FOURCC_T		encoding;									// Encoding to use for the output file. defined in userland/interface/mmal/util/mmal_il.c
	const char*			exifTags[MAX_USER_EXIF_TAGS];				// Array of pointers to tags supplied from the command line
	int numExifTags;												// Number of supplied tags
	
	CameraSettings		cameraSettings;								// Camera setup parameters
	
	MMAL_COMPONENT_T*	camera;    
	MMAL_COMPONENT_T*	encoder_component;   
	MMAL_CONNECTION_T*	encoder_connection;							// Pointer to the connection from camera to encoder
	
	MMAL_POOL_T*		encoder_pool;								// Pointer to the pool of buffers used by encoder output port
	
	void add_exif_tags();
	MMAL_STATUS_T add_exif_tag(const char* exif_tag);
	
	void setup(MMAL_COMPONENT_T* camera_);
	
};