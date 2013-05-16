#pragma once

#include "ofMain.h"

#include "interface/mmal/mmal.h"
#include "interface/mmal/mmal_logging.h"
#include "interface/mmal/mmal_buffer.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"

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

#define MAX_USER_EXIF_TAGS      32
#define MAX_EXIF_PAYLOAD_LENGTH 128


struct MMAL_PARAM_THUMBNAIL_CONFIG_T
{
	int enable;
	int width,height;
	int quality;
};
struct RASPIPREVIEW_PARAMETERS
{
	int wantPreview;                       /// Display a preview
	int wantFullScreenPreview;             /// 0 is use previewRect, non-zero to use full screen
	MMAL_RECT_T previewWindow;             /// Destination rectangle for the preview window.
	MMAL_COMPONENT_T *preview_component;   /// Pointer to the created preview display component
};

struct MMAL_PARAM_COLOURFX_T
{
	int enable;       /// Turn colourFX on or off
	int u,v;          /// U and V to use
};

struct RASPICAM_CAMERA_PARAMETERS
{
	int sharpness;             /// -100 to 100
	int contrast;              /// -100 to 100
	int brightness;            ///  0 to 100
	int saturation;            ///  -100 to 100
	int ISO;                   ///  TODO : what range?
	int videoStabilisation;    /// 0 or 1 (false or true)
	int exposureCompensation;  /// -10 to +10 ?
	MMAL_PARAM_EXPOSUREMODE_T exposureMode;
	MMAL_PARAM_EXPOSUREMETERINGMODE_T exposureMeterMode;
	MMAL_PARAM_AWBMODE_T awbMode;
	MMAL_PARAM_IMAGEFX_T imageEffect;
	MMAL_PARAMETER_IMAGEFX_PARAMETERS_T imageEffectsParameters;
	MMAL_PARAM_COLOURFX_T colourEffects;
	int rotation;              /// 0-359
	int hflip;                 /// 0 or 1
	int vflip;                 /// 0 or 1
};

struct RASPISTILL_STATE 
{
	int timeout;                        /// Time taken before frame is grabbed and app then shuts down. Units are milliseconds
	int width;                          /// Requested width of image
	int height;                         /// requested height of image
	int quality;                        /// JPEG quality setting (1-100)
	int wantRAW;                        /// Flag for whether the JPEG metadata also contains the RAW bayer image
	char *filename;                     /// filename of output file
	MMAL_PARAM_THUMBNAIL_CONFIG_T thumbnailConfig;
	int verbose;                        /// !0 if want detailed run information
	int demoMode;                       /// Run app in demo mode
	int demoInterval;                   /// Interval between camera settings changes
	MMAL_FOURCC_T encoding;             /// Encoding to use for the output file.
	const char *exifTags[MAX_USER_EXIF_TAGS]; /// Array of pointers to tags supplied from the command line
	int numExifTags;                    /// Number of supplied tags
	int timelapse;                      /// Delay between each picture in timelapse mode. If 0, disable timelapse
	
	RASPIPREVIEW_PARAMETERS preview_parameters;    /// Preview setup parameters
	RASPICAM_CAMERA_PARAMETERS camera_parameters; /// Camera setup parameters
	
	MMAL_COMPONENT_T *camera_component;    /// Pointer to the camera component
	MMAL_COMPONENT_T *encoder_component;   /// Pointer to the encoder component
	MMAL_CONNECTION_T *preview_connection; /// Pointer to the connection from camera to preview
	MMAL_CONNECTION_T *encoder_connection; /// Pointer to the connection from camera to encoder
	
	MMAL_POOL_T *encoder_pool; /// Pointer to the pool of buffers used by encoder output port
};

struct PORT_USERDATA
{
	FILE *file_handle;                   /// File handle to write buffer data to.
	VCOS_SEMAPHORE_T complete_semaphore; /// semaphore which is posted when we reach end of frame (indicates end of capture or fault)
	RASPISTILL_STATE *pstate;            /// pointer to our state in case required in callback
};

class CameraControl
{
public:
	CameraControl();
	void setup(MMAL_COMPONENT_T *camera, const RASPICAM_CAMERA_PARAMETERS *params);
};
