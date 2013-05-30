//
//  SlideShow.h
//  SlideShowApp
//
//  Created by Jason Van Cleave on 4/20/13.
//
//

#pragma once

#include "ofMain.h"

class SlideShow
{
public:
    SlideShow();
    void setup(string photosFolder);
    void update();
    void draw();
    vector<ofImage> images;
    ofImage* currentImage;
    ofImage* previousImage;
    int counter;
    int transitionColor;
    int waitCounter;
	void addPhoto(string photoPath);
	bool isReloading;
};
