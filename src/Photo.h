#pragma once

#include "ofMain.h"
#include "interface/mmal/mmal.h"
#include "interface/mmal/mmal_logging.h"
#include "interface/mmal/mmal_buffer.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"
#include "ThumbnailConfig.h"
#include "CameraSettings.h"

#define MAX_USER_EXIF_TAGS      32
#define MAX_EXIF_PAYLOAD_LENGTH 128

class Photo
{
public:
	Photo();
	int timeout;                        /// Time taken before frame is grabbed and app then shuts down. Units are milliseconds
	int width;                          /// Requested width of image
	int height;                         /// requested height of image
	int quality;                        /// JPEG quality setting (1-100)
	int wantRAW;                        /// Flag for whether the JPEG metadata also contains the RAW bayer image
	char *filename;                     /// filename of output file
	ThumbnailConfig	thumbnailConfig;
	int verbose;                        /// !0 if want detailed run information
	int demoMode;                       /// Run app in demo mode
	int demoInterval;                   /// Interval between camera settings changes
	MMAL_FOURCC_T encoding;             /// Encoding to use for the output file.
	const char *exifTags[MAX_USER_EXIF_TAGS]; /// Array of pointers to tags supplied from the command line
	int numExifTags;                    /// Number of supplied tags
	int timelapse;                      /// Delay between each picture in timelapse mode. If 0, disable timelapse
	
	CameraSettings camera_parameters; /// Camera setup parameters
	
	MMAL_COMPONENT_T *camera_component;    /// Pointer to the camera component
	MMAL_COMPONENT_T *encoder_component;   /// Pointer to the encoder component
	MMAL_CONNECTION_T *preview_connection; /// Pointer to the connection from camera to preview
	MMAL_CONNECTION_T *encoder_connection; /// Pointer to the connection from camera to encoder
	
	MMAL_POOL_T *encoder_pool; /// Pointer to the pool of buffers used by encoder output port
	
	void add_exif_tags();
	MMAL_STATUS_T add_exif_tag(const char *exif_tag);
};