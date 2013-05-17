/*
 *  Photo.cpp
 *  openFrameworksLib
 *
 *  Created by jason van cleave on 5/17/13.
 *  Copyright 2013 jasonvancleave.com. All rights reserved.
 *
 */

#include "Photo.h"

Photo::Photo()
{

}

/**
 * Add an exif tag to the capture
 *
 * @param state Pointer to state control struct
 * @param exif_tag String containing a "key=value" pair.
 * @return  Returns a MMAL_STATUS_T giving result of operation
 */
MMAL_STATUS_T Photo::add_exif_tag(const char *exif_tag)
{
	MMAL_STATUS_T status;
	MMAL_PARAMETER_EXIF_T *exif_param = (MMAL_PARAMETER_EXIF_T*)calloc(sizeof(MMAL_PARAMETER_EXIF_T) + MAX_EXIF_PAYLOAD_LENGTH, 1);
	
	//vcos_assert(this);
	//vcos_assert(encoder_component);
	
	// Check to see if the tag is present or is indeed a key=value pair.
	if (!exif_tag || strchr(exif_tag, '=') == NULL || strlen(exif_tag) > MAX_EXIF_PAYLOAD_LENGTH-1)
		return MMAL_EINVAL;
	
	exif_param->hdr.id = MMAL_PARAMETER_EXIF;
	
	strncpy((char*)exif_param->data, exif_tag, MAX_EXIF_PAYLOAD_LENGTH-1);
	
	exif_param->hdr.size = sizeof(MMAL_PARAMETER_EXIF_T) + strlen((char*)exif_param->data);
	
	status = mmal_port_parameter_set(encoder_component->output[0], &exif_param->hdr);
	
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
void Photo::add_exif_tags()
{
	time_t rawtime;
	struct tm *timeinfo;
	char time_buf[32];
	char exif_buf[128];
	int i;
	
	add_exif_tag("IFD0.Model=RP_OV5647");
	add_exif_tag("IFD0.Make=RaspberryPi");
	
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
	add_exif_tag(exif_buf);
	
	snprintf(exif_buf, sizeof(exif_buf), "EXIF.DateTimeOriginal=%s", time_buf);
	add_exif_tag(exif_buf);
	
	snprintf(exif_buf, sizeof(exif_buf), "IFD0.DateTime=%s", time_buf);
	add_exif_tag(exif_buf);
	
	// Now send any user supplied tags
	
	for (i=0;i<numExifTags && i < MAX_USER_EXIF_TAGS; i++)
	{
		if (exifTags[i])
		{
			add_exif_tag(exifTags[i]);
		}
	}
}
