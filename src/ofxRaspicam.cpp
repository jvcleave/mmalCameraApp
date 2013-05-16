/*
 *  ofxRaspicam.cpp
 *  openFrameworksLib
 *
 *  Created by jason van cleave on 5/16/13.
 *  Copyright 2013 jasonvancleave.com. All rights reserved.
 *
 */

#include "ofxRaspicam.h"

/**
 *  buffer header callback function for camera control
 *
 *  No actions taken in current version
 *
 * @param port Pointer to port from which callback originated
 * @param buffer mmal buffer header pointer
 */
static void camera_control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
	if (buffer->cmd == MMAL_EVENT_PARAMETER_CHANGED)
	{
	}
	else
	{
		//ofLogVerbose() << "Received unexpected camera control callback event, 0x%08x", buffer->cmd);
	}
	
	mmal_buffer_header_release(buffer);
}

/**
 * Connect two specific ports together
 *
 * @param output_port Pointer the output port
 * @param input_port Pointer the input port
 * @param Pointer to a mmal connection pointer, reassigned if function successful
 * @return Returns a MMAL_STATUS_T giving result of operation
 *
 */
static MMAL_STATUS_T connect_ports(MMAL_PORT_T *output_port, MMAL_PORT_T *input_port, MMAL_CONNECTION_T **connection)
{
	MMAL_STATUS_T status;
	
	status =  mmal_connection_create(connection, output_port, input_port, MMAL_CONNECTION_FLAG_TUNNELLING | MMAL_CONNECTION_FLAG_ALLOCATION_ON_INPUT);
	
	if (status == MMAL_SUCCESS)
	{
		status =  mmal_connection_enable(*connection);
		if (status != MMAL_SUCCESS)
			mmal_connection_destroy(*connection);
	}
	
	return status;
}

/**
 *  buffer header callback function for encoder
 *
 *  Callback will dump buffer data to the specific file
 *
 * @param port Pointer to port from which callback originated
 * @param buffer mmal buffer header pointer
 */
static void encoder_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
	int complete = 0;
	
	// We pass our file handle and other stuff in via the userdata field.
	
	PORT_USERDATA *pData = (PORT_USERDATA *)port->userdata;
	
	if (pData)
	{
		if (buffer->length && pData->file_handle)
		{
			mmal_buffer_header_mem_lock(buffer);
			
			fwrite(buffer->data, 1, buffer->length, pData->file_handle);
			
			mmal_buffer_header_mem_unlock(buffer);
		}
		
		// Now flag if we have completed
		if (buffer->flags & (MMAL_BUFFER_HEADER_FLAG_FRAME_END | MMAL_BUFFER_HEADER_FLAG_TRANSMISSION_FAILED))
			complete = 1;
	}
	else
	{
		vcos_log_error("Received a encoder buffer callback with no state");
	}
	
	// release buffer back to the pool
	mmal_buffer_header_release(buffer);
	
	// and send one back to the port (if still open)
	if (port->is_enabled)
	{
		MMAL_STATUS_T status;
		MMAL_BUFFER_HEADER_T *new_buffer;
		
		new_buffer = mmal_queue_get(pData->pstate->encoder_pool->queue);
		
		if (new_buffer)
		{
			status = mmal_port_send_buffer(port, new_buffer);
		}
		if (!new_buffer || status != MMAL_SUCCESS)
			vcos_log_error("Unable to return a buffer to the encoder port");
	}
	
	if (complete)
		vcos_semaphore_post(&(pData->complete_semaphore));
	
}
/**
 * Add an exif tag to the capture
 *
 * @param state Pointer to state control struct
 * @param exif_tag String containing a "key=value" pair.
 * @return  Returns a MMAL_STATUS_T giving result of operation
 */
static MMAL_STATUS_T add_exif_tag(RASPISTILL_STATE *state, const char *exif_tag)
{
	MMAL_STATUS_T status;
	MMAL_PARAMETER_EXIF_T *exif_param = (MMAL_PARAMETER_EXIF_T*)calloc(sizeof(MMAL_PARAMETER_EXIF_T) + MAX_EXIF_PAYLOAD_LENGTH, 1);
	
	vcos_assert(state);
	vcos_assert(state->encoder_component);
	
	// Check to see if the tag is present or is indeed a key=value pair.
	if (!exif_tag || strchr(exif_tag, '=') == NULL || strlen(exif_tag) > MAX_EXIF_PAYLOAD_LENGTH-1)
		return MMAL_EINVAL;
	
	exif_param->hdr.id = MMAL_PARAMETER_EXIF;
	
	strncpy((char*)exif_param->data, exif_tag, MAX_EXIF_PAYLOAD_LENGTH-1);
	
	exif_param->hdr.size = sizeof(MMAL_PARAMETER_EXIF_T) + strlen((char*)exif_param->data);
	
	status = mmal_port_parameter_set(state->encoder_component->output[0], &exif_param->hdr);
	
	free(exif_param);
	
	return status;
}

/**
 * Add a basic set of EXIF tags to the capture
 * Make, Time etc
 *
 * @param state Pointer to state control struct
 *
 */
static void add_exif_tags(RASPISTILL_STATE *state)
{
	time_t rawtime;
	struct tm *timeinfo;
	char time_buf[32];
	char exif_buf[128];
	int i;
	
	add_exif_tag(state, "IFD0.Model=RP_OV5647");
	add_exif_tag(state, "IFD0.Make=RaspberryPi");
	
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	
	snprintf(time_buf, sizeof(time_buf),
			 "%04d:%02d:%02d:%02d:%02d:%02d",
			 timeinfo->tm_year+1900,
			 timeinfo->tm_mon+1,
			 timeinfo->tm_mday,
			 timeinfo->tm_hour,
			 timeinfo->tm_min,
			 timeinfo->tm_sec);
	
	snprintf(exif_buf, sizeof(exif_buf), "EXIF.DateTimeDigitized=%s", time_buf);
	add_exif_tag(state, exif_buf);
	
	snprintf(exif_buf, sizeof(exif_buf), "EXIF.DateTimeOriginal=%s", time_buf);
	add_exif_tag(state, exif_buf);
	
	snprintf(exif_buf, sizeof(exif_buf), "IFD0.DateTime=%s", time_buf);
	add_exif_tag(state, exif_buf);
	
	// Now send any user supplied tags
	
	for (i=0;i<state->numExifTags && i < MAX_USER_EXIF_TAGS;i++)
	{
		if (state->exifTags[i])
		{
			add_exif_tag(state, state->exifTags[i]);
		}
	}
}
/**
 * Convert a MMAL status return value to a simple boolean of success
 * ALso displays a fault if code is not success
 *
 * @param status The error code to convert
 * @return 0 if status is sucess, 1 otherwise
 */
int mmal_status_to_int(MMAL_STATUS_T status)
{
	if (status == MMAL_SUCCESS)
		return 0;
	else
	{
		switch (status)
		{
			case MMAL_ENOMEM :   ofLogVerbose() << "Out of memory"; break;
			case MMAL_ENOSPC :   ofLogVerbose() << "Out of resources (other than memory)"; break;
			case MMAL_EINVAL:    ofLogVerbose() << "Argument is invalid"; break;
			case MMAL_ENOSYS :   ofLogVerbose() << "Function not implemented"; break;
			case MMAL_ENOENT :   ofLogVerbose() << "No such file or directory"; break;
			case MMAL_ENXIO :    ofLogVerbose() << "No such device or address"; break;
			case MMAL_EIO :      ofLogVerbose() << "I/O error"; break;
			case MMAL_ESPIPE :   ofLogVerbose() << "Illegal seek"; break;
			case MMAL_ECORRUPT : ofLogVerbose() << "Data is corrupt \attention FIXME: not POSIX"; break;
			case MMAL_ENOTREADY :ofLogVerbose() << "Component is not ready \attention FIXME: not POSIX"; break;
			case MMAL_ECONFIG :  ofLogVerbose() << "Component is not configured \attention FIXME: not POSIX"; break;
			case MMAL_EISCONN :  ofLogVerbose() << "Port is already connected "; break;
			case MMAL_ENOTCONN : ofLogVerbose() << "Port is disconnected"; break;
			case MMAL_EAGAIN :   ofLogVerbose() << "Resource temporarily unavailable. Try again later"; break;
			case MMAL_EFAULT :   ofLogVerbose() << "Bad address"; break;
			default :            ofLogVerbose() << "Unknown status error"; break;
		}
		
		return 1;
	}
}
ofxRaspicam::ofxRaspicam()
{
	camera_preview_port = NULL;
	camera_video_port = NULL;
	camera_still_port = NULL;
	preview_input_port = NULL;
	encoder_input_port = NULL;
	encoder_output_port = NULL;
	camera = NULL;
	encoder = NULL;
}

void ofxRaspicam::setup()
{
	
	MMAL_STATUS_T status = MMAL_SUCCESS;

	
	bcm_host_init();
	
	state.timeout = 5000; // 5s delay before take image
	state.width = 2592;
	state.height = 1944;
	state.quality = 85;
	state.wantRAW = 0;
	state.filename = NULL;
	state.verbose = 0;
	state.thumbnailConfig.enable = 1;
	state.thumbnailConfig.width = 64;
	state.thumbnailConfig.height = 48;
	state.thumbnailConfig.quality = 35;
	state.demoMode = 0;
	state.demoInterval = 250; // ms
	state.camera_component = NULL;
	state.encoder_component = NULL;
	state.preview_connection = NULL;
	state.encoder_connection = NULL;
	state.encoder_pool = NULL;
	state.encoding = MMAL_ENCODING_JPEG;
	state.numExifTags = 0;
	state.timelapse = 0;
	
	state.camera_parameters.sharpness = 0;
	state.camera_parameters.contrast = 0;
	state.camera_parameters.brightness = 50;
	state.camera_parameters.saturation = 0;
	state.camera_parameters.ISO = 400;
	state.camera_parameters.videoStabilisation = 0;
	state.camera_parameters.exposureCompensation = 0;
	state.camera_parameters.exposureMode = MMAL_PARAM_EXPOSUREMODE_AUTO;
	state.camera_parameters.exposureMeterMode = MMAL_PARAM_EXPOSUREMETERINGMODE_AVERAGE;
	state.camera_parameters.awbMode = MMAL_PARAM_AWBMODE_AUTO;
	state.camera_parameters.imageEffect = MMAL_PARAM_IMAGEFX_NONE;
	state.camera_parameters.colourEffects.enable = 0;
	state.camera_parameters.colourEffects.u = 128;
	state.camera_parameters.colourEffects.v = 128;
	state.camera_parameters.rotation = 0;
	state.camera_parameters.hflip = state.camera_parameters.vflip = 0;
	create_camera_component();
	create_encoder_component();
	
	camera_preview_port = state.camera_component->output[MMAL_CAMERA_PREVIEW_PORT];
	camera_video_port   = state.camera_component->output[MMAL_CAMERA_VIDEO_PORT];
	camera_still_port   = state.camera_component->output[MMAL_CAMERA_CAPTURE_PORT];
	preview_input_port  = state.preview_parameters.preview_component->input[0];
	encoder_input_port  = state.encoder_component->input[0];
	encoder_output_port = state.encoder_component->output[0];
	
	if (status == MMAL_SUCCESS)
	{
		VCOS_STATUS_T vcos_status;
		
		ofLogVerbose() << "Connecting camera stills port to encoder input port";
            
		
		// Now connect the camera to the encoder
		status = connect_ports(camera_still_port, encoder_input_port, &state.encoder_connection);
		
		if (status != MMAL_SUCCESS)
		{
            ofLogVerbose() << "Failed to connect camera video port to encoder input";
            //goto error;
		}
		
		// Set up our userdata - this is passed though to the callback where we need the information.
		// Null until we open our filename
		callback_data.file_handle = NULL;
		callback_data.pstate = &state;
		vcos_status = vcos_semaphore_create(&callback_data.complete_semaphore, "RaspiStill-sem", 0);
		
		vcos_assert(vcos_status == VCOS_SUCCESS);
		
		encoder_output_port->userdata = (struct MMAL_PORT_USERDATA_T *)&callback_data;
		
		ofLogVerbose() << "Enabling encoder output port";
            
		
		// Enable the encoder output port and tell it its callback function
		status = mmal_port_enable(encoder_output_port, encoder_buffer_callback);
		
		if (status != MMAL_SUCCESS)
		{
            ofLogVerbose() << "Failed to setup encoder output";
            //goto error;
		}
		
		int num_iterations =  state.timelapse ? state.timeout / state.timelapse : 1;
		int frame;
		FILE *output_file = NULL;
		
		for (frame = 1;frame<=num_iterations; frame++)
		{
			if (state.timelapse)
				vcos_sleep(state.timelapse);
			else
				vcos_sleep(state.timeout);
			
			// Open the file
			if (state.filename)
			{
				if (state.filename[0] == '-')
				{
					output_file = stdout;
					
					// Ensure we don't upset the output stream with diagnostics/info
					state.verbose = 0;
				}
				else
				{
					char *use_filename = state.filename;
					
					if (state.timelapse)
						asprintf(&use_filename, state.filename, frame);
					
					if (state.verbose)
						ofLogVerbose() << "Opening output file" << use_filename;
					
					output_file = fopen(use_filename, "wb");
					
					if (!output_file)
					{
						// Notify user, carry on but discarding encoded output buffers
						ofLogVerbose() << "Error opening output file";
						//vcos_log_error("%s: Error opening output file: %s\nNo output file will be generated\n", __func__, use_filename);
					}
					
					// asprintf used in timelaspe mode allocates its own memory which we need to free
					if (state.timelapse)
						free(use_filename);
				}
				
				add_exif_tags(&state);
				
				callback_data.file_handle = output_file;
			}
			
			// We only capture if a filename was specified and it opened
			if (output_file)
			{
				// Send all the buffers to the encoder output port
				int num = mmal_queue_length(state.encoder_pool->queue);
				int q;
				
				for (q=0;q<num;q++)
				{
					MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(state.encoder_pool->queue);
					
					if (!buffer)
					{
						ofLogVerbose() << "Unable to get a required buffer " << q << " from pool queue";
					}
					
					if (mmal_port_send_buffer(encoder_output_port, buffer)!= MMAL_SUCCESS)
					{
						ofLogVerbose() << "Unable to send a buffer to encoder output port " << q;
					}
				}
				
				ofLogVerbose() << "Starting capture " << frame;
				
				
				if (mmal_port_parameter_set_boolean(camera_still_port, MMAL_PARAMETER_CAPTURE, 1) != MMAL_SUCCESS)
				{
					ofLogVerbose() << "Failed to start capture";
				}
				else
				{
					// Wait for capture to complete
					// For some reason using vcos_semaphore_wait_timeout sometimes returns immediately with bad parameter error
					// even though it appears to be all correct, so reverting to untimed one until figure out why its erratic
					vcos_semaphore_wait(&callback_data.complete_semaphore);
					ofLogVerbose() << "Finished capture " << frame;
				}
				
				// Ensure we don't die if get callback with no open file
				callback_data.file_handle = NULL;
				
				if (output_file != stdout)
					fclose(output_file);
			}
			
		} // end for (frame)
		
		vcos_semaphore_delete(&callback_data.complete_semaphore);
		
	}
	else
	{
		mmal_status_to_int(status);
		ofLogVerbose() << "Failed to connect camera to preview";
	}
}

void ofxRaspicam::create_camera_component()
{
	
	MMAL_ES_FORMAT_T *format;
	MMAL_STATUS_T status;
	
	/* Create the component */
	status = mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &camera);
	
	if (status != MMAL_SUCCESS)
	{
		ofLogVerbose() << "Failed to create camera component";
		//goto error;
	}
	
	if (!camera->output_num)
	{
		ofLogVerbose() << "Camera doesn't have output ports";
		//goto error;
	}
	
	camera_preview_port = camera->output[MMAL_CAMERA_PREVIEW_PORT];
	camera_video_port = camera->output[MMAL_CAMERA_VIDEO_PORT];
	camera_still_port = camera->output[MMAL_CAMERA_CAPTURE_PORT];
	
	// Enable the camera, and tell it its control callback function
	status = mmal_port_enable(camera->control, camera_control_callback);
	
	if (status)
	{
		ofLogVerbose() << "Unable to enable control port : error" <<  status;
		//goto error;
	}
	
	MMAL_PARAMETER_CAMERA_CONFIG_T cam_config;
	cam_config.max_stills_w = state.width;
	cam_config.max_stills_h = state.height;
	cam_config.stills_yuv422 = 0;
	cam_config.one_shot_stills = 1;
	cam_config.max_preview_video_w = state.preview_parameters.previewWindow.width;
	cam_config.max_preview_video_h = state.preview_parameters.previewWindow.height;
	cam_config.num_preview_video_frames = 3;
	cam_config.stills_capture_circular_buffer_height = 0;
	cam_config.fast_preview_resume = 0;
	cam_config.use_stc_timestamp = MMAL_PARAM_TIMESTAMP_MODE_RESET_STC;
	mmal_port_parameter_set(camera->control, &cam_config.hdr);
	/*{
		MMAL_PARAMETER_CAMERA_CONFIG_T cam_config =
		{
			{ MMAL_PARAMETER_CAMERA_CONFIG, sizeof(cam_config) },
			.max_stills_w = state.width,
			.max_stills_h = state.height,
			cam_config.stills_yuv422 = 0,
			cam_config.one_shot_stills = 1,
			cam_config.max_preview_video_w = state.preview_parameters.previewWindow.width,
			cam_config.max_preview_video_h = state.preview_parameters.previewWindow.height,
			cam_config.num_preview_video_frames = 3,
			cam_config.stills_capture_circular_buffer_height = 0,
			cam_config.fast_preview_resume = 0,
			cam_config.use_stc_timestamp = MMAL_PARAM_TIMESTAMP_MODE_RESET_STC
		};
		mmal_port_parameter_set(camera->control, &cam_config.hdr);
	}*/
	
	cameraController.setup(camera, &state.camera_parameters);
	//raspicamcontrol_set_all_parameters(camera, &state.camera_parameters);
	
	// Now set up the port formats
	
	format = camera_preview_port->format;
	
	format->encoding = MMAL_ENCODING_OPAQUE;
	format->encoding_variant = MMAL_ENCODING_I420;
	
	format->es->video.width = state.preview_parameters.previewWindow.width;
	format->es->video.height = state.preview_parameters.previewWindow.height;
	format->es->video.crop.x = 0;
	format->es->video.crop.y = 0;
	format->es->video.crop.width = state.preview_parameters.previewWindow.width;
	format->es->video.crop.height = state.preview_parameters.previewWindow.height;
	format->es->video.frame_rate.num = PREVIEW_FRAME_RATE_NUM;
	format->es->video.frame_rate.den = PREVIEW_FRAME_RATE_DEN;
	
	status = mmal_port_format_commit(camera_preview_port);
	
	if (status)
	{
		ofLogVerbose() << "camera viewfinder format couldn't be set";
		//goto error;
	}
	
	// Set the same format on the video  port (which we dont use here)
	mmal_format_full_copy(camera_video_port->format, format);
	status = mmal_port_format_commit(camera_video_port);
	
	if (status)
	{
		ofLogVerbose() << "camera video format couldn't be set";
		//goto error;
	}
	
	// Ensure there are enough buffers to avoid dropping frames
	if (camera_video_port->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
		camera_video_port->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;
	
	format = camera_still_port->format;
	
	// Set our stills format on the stills (for encoder) port
	format->encoding = MMAL_ENCODING_OPAQUE;
	format->es->video.width = state.width;
	format->es->video.height = state.height;
	format->es->video.crop.x = 0;
	format->es->video.crop.y = 0;
	format->es->video.crop.width = state.width;
	format->es->video.crop.height = state.height;
	format->es->video.frame_rate.num = STILLS_FRAME_RATE_NUM;
	format->es->video.frame_rate.den = STILLS_FRAME_RATE_DEN;
	
	status = mmal_port_format_commit(camera_still_port);
	
	if (status)
	{
		ofLogVerbose() << "camera still format couldn't be set";
		//goto error;
	}
	
	/* Ensure there are enough buffers to avoid dropping frames */
	if (camera_still_port->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
		camera_still_port->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;
	
	/* Enable component */
	status = mmal_component_enable(camera);
	
	if (status)
	{
		ofLogVerbose() << "camera component couldn't be enabled";
		//goto error;
	}
	
	if (state.wantRAW)
	{
		if (mmal_port_parameter_set_boolean(camera_still_port, MMAL_PARAMETER_ENABLE_RAW_CAPTURE, 1) != MMAL_SUCCESS)
		{
			ofLogVerbose() << "RAW was requested, but failed to enable";
			
			// Continue on and take picture without.
		}
	}
	
	state.camera_component = camera;
	
	ofLogVerbose() << "Camera component done";
	
	//return camera;
	
error:
	
	if (camera)
	{
		mmal_component_destroy(camera);
	}
	
	//return NULL;
}

void ofxRaspicam::create_encoder_component()
{
	MMAL_STATUS_T status;
	MMAL_POOL_T *pool;
	
	status = mmal_component_create(MMAL_COMPONENT_DEFAULT_IMAGE_ENCODER, &encoder);
	
	if (status != MMAL_SUCCESS)
	{
		ofLogVerbose() << "Unable to create JPEG encoder component";
		//goto error;
	}
	
	if (!encoder->input_num || !encoder->output_num)
	{
		ofLogVerbose() << "JPEG encoder doesn't have input/output ports";
		//goto error;
	}
	
	encoder_input_port = encoder->input[0];
	encoder_output_port = encoder->output[0];
	
	// We want same format on input and output
	mmal_format_copy(encoder_output_port->format, encoder_input_port->format);
	
	// Specify out output format
	encoder_output_port->format->encoding = state.encoding;
	
	encoder_output_port->buffer_size = encoder_output_port->buffer_size_recommended;
	
	if (encoder_output_port->buffer_size < encoder_output_port->buffer_size_min)
		encoder_output_port->buffer_size = encoder_output_port->buffer_size_min;
	
	encoder_output_port->buffer_num = encoder_output_port->buffer_num_recommended;
	
	if (encoder_output_port->buffer_num < encoder_output_port->buffer_num_min)
		encoder_output_port->buffer_num = encoder_output_port->buffer_num_min;
	
	// Commit the port changes to the output port
	status = mmal_port_format_commit(encoder_output_port);
	
	if (status != MMAL_SUCCESS)
	{
		ofLogVerbose() << "Unable to set format on video encoder output port";
		//goto error;
	}
	
	// Set the JPEG quality level
	status = mmal_port_parameter_set_uint32(encoder_output_port, MMAL_PARAMETER_JPEG_Q_FACTOR, state.quality);
	
	if (status != MMAL_SUCCESS)
	{
		ofLogVerbose() << "Unable to set JPEG quality";
		//goto error;
	}
	MMAL_PARAMETER_THUMBNAIL_CONFIG_T param_thumb;
	param_thumb.enable = 1;
	param_thumb.width = state.thumbnailConfig.width;
	param_thumb.height = state.thumbnailConfig.height;
	param_thumb.quality = state.thumbnailConfig.quality;
	status = mmal_port_parameter_set(encoder->control, &param_thumb.hdr);


	
	//  Enable component
	status = mmal_component_enable(encoder);
	
	if (status)
	{
		ofLogVerbose() << "Unable to enable video encoder component";
		//goto error;
	}
	
	/* Create pool of buffer headers for the output port to consume */
	pool = mmal_port_pool_create(encoder_output_port, encoder_output_port->buffer_num, encoder_output_port->buffer_size);
	
	if (!pool)
	{
		ofLogVerbose() << "Failed to create buffer header pool for encoder output port " << encoder_output_port->name;
	}
	
	state.encoder_pool = pool;
	state.encoder_component = encoder;
	
	ofLogVerbose() << "Encoder component done";
	
	//return encoder;
	
error:
	if (encoder)
		mmal_component_destroy(encoder);
	
	//return 0;
	
}
