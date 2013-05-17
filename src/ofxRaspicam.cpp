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
	camera_still_port = NULL;
	encoder_input_port = NULL;
	encoder_output_port = NULL;
	camera = NULL;
	encoder = NULL;
}

void ofxRaspicam::setup()
{
	
	MMAL_STATUS_T status = MMAL_SUCCESS;

	
	bcm_host_init();

	create_camera_component();
	create_encoder_component();
	
	camera_still_port   = state.camera_component->output[MMAL_CAMERA_CAPTURE_PORT];
	encoder_input_port  = state.encoder_component->input[0];
	encoder_output_port = state.encoder_component->output[0];
	
	VCOS_STATUS_T vcos_status;
	
	ofLogVerbose() << "Connecting camera stills port to encoder input port";
		
	
	// Now connect the camera to the encoder
	status = connect_ports(camera_still_port, encoder_input_port, &state.encoder_connection);
	
	if (status != MMAL_SUCCESS)
	{
		ofLogVerbose() << "connect camera video port to encoder input FAIL";
	}else 
	{
		ofLogVerbose() << "connect camera video port to encoder input PASS";

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
		ofLogVerbose() << "Setup encoder output FAIL, error: " << status;
	}else 
	{
		ofLogVerbose() << "Setup encoder output PASS";
	}

	
	int frame;
	FILE *output_file = NULL;
		
	vcos_sleep(state.timeout);
	
	// Open the file
	string fileName = ofGetTimestampString()+".jpg";
	char *toChar = const_cast<char*> ( fileName.c_str() );
	state.filename = toChar;

	ofLogVerbose() << "Opening output file" << fileName;
	
	output_file = fopen(state.filename, "wb");
	
	if (!output_file)
	{
		// Notify user, carry on but discarding encoded output buffers
		ofLogVerbose() << "Error opening output file";
	}
	state.add_exif_tags();
	
	callback_data.file_handle = output_file;
	
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
		
		fclose(output_file);
	}
	
	vcos_semaphore_delete(&callback_data.complete_semaphore);
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
		
	}
	
	if (!camera->output_num)
	{
		ofLogVerbose() << "Camera doesn't have output ports";
		
	}else {
		ofLogVerbose() << "camera->output_num: " << camera->output_num;
	}

	
	camera_still_port = camera->output[MMAL_CAMERA_CAPTURE_PORT];
	
	// Enable the camera, and tell it its control callback function
	status = mmal_port_enable(camera->control, camera_control_callback);
	
	if (status)
	{
		ofLogVerbose() << "Enable control port FAIL: error" <<  status;
	}
	else 
	{
		ofLogVerbose() << "Enable control port : PASS " <<  status;
	}

	
	MMAL_PARAMETER_CAMERA_CONFIG_T cam_config;
	cam_config.max_stills_w = state.width;
	cam_config.max_stills_h = state.height;
	cam_config.stills_yuv422 = 0;
	cam_config.one_shot_stills = 1;

	cam_config.max_preview_video_w = ofGetWidth();
	cam_config.max_preview_video_h = ofGetHeight();
	cam_config.num_preview_video_frames = 3;
	cam_config.stills_capture_circular_buffer_height = 0;
	cam_config.fast_preview_resume = 0;
	cam_config.use_stc_timestamp = MMAL_PARAM_TIMESTAMP_MODE_RESET_STC;
	
	//raspicamcontrol_set_all_parameters(camera, &state.camera_parameters);
	
	// Now set up the port formats
	
	
	
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
		
	}
	
	/* Ensure there are enough buffers to avoid dropping frames */
	if (camera_still_port->buffer_num < OUTPUT_BUFFERS_NUM)
	{
		camera_still_port->buffer_num = OUTPUT_BUFFERS_NUM;
	}
	
	/* Enable component */
	status = mmal_component_enable(camera);
	
	if (status)
	{
		ofLogVerbose() << "camera component enable FAIL";
	}else 
	{
		ofLogVerbose() << "camera component enable PASS";
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
}

ofxRaspicam::~ofxRaspicam()
{
	ofLogVerbose() << "~ofxRaspicam";
	if (camera)
	{
		mmal_component_destroy(camera);
		ofLogVerbose() << "camera DESTROYED";
	}
	if (encoder)
	{
		mmal_component_destroy(encoder);
		ofLogVerbose() << "encoder DESTROYED";
	}
}

void ofxRaspicam::create_encoder_component()
{
	ofLogVerbose() << "create_encoder_component START";
	MMAL_STATUS_T status;
	MMAL_POOL_T *pool;
	
	status = mmal_component_create(MMAL_COMPONENT_DEFAULT_IMAGE_ENCODER, &encoder);
	
	if (status != MMAL_SUCCESS)
	{
		ofLogVerbose() << "create JPEG encoder component FAIL error: " << status;
	}else 
	{
		ofLogVerbose() << "create JPEG encoder component PASS";
	}

	
	if (!encoder->input_num || !encoder->output_num)
	{
		ofLogVerbose() << "JPEG encoder doesn't have input/output ports";
	}
	
	encoder_input_port = encoder->input[0];
	encoder_output_port = encoder->output[0];
	
	// We want same format on input and output
	mmal_format_copy(encoder_output_port->format, encoder_input_port->format);
	
	// Specify out output format
	encoder_output_port->format->encoding = state.encoding;
	
	encoder_output_port->buffer_size = encoder_output_port->buffer_size_recommended;
	
	if (encoder_output_port->buffer_size < encoder_output_port->buffer_size_min)
	{
		encoder_output_port->buffer_size = encoder_output_port->buffer_size_min;
	}
	
	encoder_output_port->buffer_num = encoder_output_port->buffer_num_recommended;
	
	if (encoder_output_port->buffer_num < encoder_output_port->buffer_num_min)
	{
		encoder_output_port->buffer_num = encoder_output_port->buffer_num_min;
	}
	
	// Commit the port changes to the output port
	status = mmal_port_format_commit(encoder_output_port);
	
	if (status != MMAL_SUCCESS)
	{
		ofLogVerbose() << "set format on video encoder output port FAIL, error: " << status;
	}else 
	{
		ofLogVerbose() << "set format on video encoder output port PASS";

	}

	
	// Set the JPEG quality level
	status = mmal_port_parameter_set_uint32(encoder_output_port, MMAL_PARAMETER_JPEG_Q_FACTOR, state.quality);
	
	if (status != MMAL_SUCCESS)
	{
		ofLogVerbose() << "Set JPEG quality FAIL, error: " << status;
	}else 
	{
		ofLogVerbose() << "Set JPEG quality PASS";

	}

	/*MMAL_PARAMETER_THUMBNAIL_CONFIG_T param_thumb;
	param_thumb.enable = 1;
	param_thumb.width = state.thumbnailConfig.width;
	param_thumb.height = state.thumbnailConfig.height;
	param_thumb.quality = state.thumbnailConfig.quality;
	status = mmal_port_parameter_set(encoder->control, &param_thumb.hdr);
	if (status)
	{
		ofLogVerbose() << "mmal_port_parameter_set FAIL, error: " << status;
	}else 
	{
		ofLogVerbose() << "mmal_port_parameter_set PASS";
		
	}*/

	
	//  Enable component
	status = mmal_component_enable(encoder);
	
	if (status)
	{
		ofLogVerbose() << "Enable video encoder component FAIL, error: " << status;
	}else 
	{
		ofLogVerbose() << "Enable video encoder component PASS";
	}

	
	/* Create pool of buffer headers for the output port to consume */
	pool = mmal_port_pool_create(encoder_output_port, encoder_output_port->buffer_num, encoder_output_port->buffer_size);
	
	if (!pool)
	{
		ofLogVerbose() << "Failed to create buffer header pool for encoder output port " << encoder_output_port->name;
	}else 
	{
		ofLogVerbose() << "pool creation PASS";
	}

	
	state.encoder_pool = pool;
	state.encoder_component = encoder;
	
	ofLogVerbose() << "Encoder component done";

}
