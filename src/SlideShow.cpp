//
//  SlideShow.cpp
//  SlideShowApp
//
//  Created by Jason Van Cleave on 4/20/13.
//
//
#include "SlideShow.h"



SlideShow::SlideShow()
{
	isReloading = false;
}

void SlideShow::setup(string photosFolder)
{
    ofDirectory photosDirectory(photosFolder);
    photosDirectory.listDir();
	//photosDirectory.sort();
    ofLogVerbose() << photosDirectory.path();
    vector<ofFile> files = photosDirectory.getFiles();
    ofLogVerbose() << "files.size() :" << files.size();
    
    int numFiles = files.size();
    int maxNumFiles = 4;
    for (int i=0; i<files.size(); i++)
    {
        if (images.size()<maxNumFiles)
        {
            ofFile file = files[ofRandom(files.size())];
            if (file.getExtension() == "jpg" && ofIsStringInString(file.path(), "half"))
            {
                addPhoto(file.path());
            }
        }
        
        
        
    }
    ofLogVerbose() << "images.size() " << images.size();
    counter = 0;
    currentImage = &images[counter];
    previousImage = NULL;
    waitCounter = 0;

}
void SlideShow::addPhoto(string photoPath)
{
	isReloading = true;
	ofImage image;
	if(image.loadImage(photoPath))
	{
		ofLogVerbose() << "loading :" << photoPath;
		//image.resize(ofGetWidth(), ofGetHeight());
		images.push_back(image);
	}
	isReloading = false;
	counter = 0;
    currentImage = &images[counter];
    previousImage = NULL;
    waitCounter = 0;
}
void SlideShow::update()
{
    if (isReloading) {
		return;
	}
    if (transitionColor+1<=255)
    {
        transitionColor++;
        
    }else
    {
        if (transitionColor == 255)
        {
            if (waitCounter<30)
            {
                waitCounter++;
                //ofLogVerbose() << "waitcounter";
            }else
            {
                waitCounter=0;
            }
        }
        
        if(waitCounter == 0)
        {
            transitionColor =0;
            if (counter+1 < images.size())
            {
                counter++;
            }else
            {
                counter = 0;
            }
            previousImage = currentImage;
            currentImage = &images[counter];
        }
    }
}

void SlideShow::draw()
{
    ofEnableAlphaBlending();
    if (previousImage != NULL)
    {
        ofSetColor(255, 255, 255, 255-transitionColor);
        previousImage->draw(0, 0, ofGetWidth(), ofGetHeight());
    }
    ofSetColor(255, 255, 255, transitionColor);
    currentImage->draw(0, 0, ofGetWidth(), ofGetHeight());
    ofDisableAlphaBlending();
    ofDrawBitmapStringHighlight(ofToString(waitCounter), 20, 20, ofColor::black, ofColor::yellow);
}





