#pragma once

#include "ofMain.h"
#include "ofxRaspicam.h"
#include "ConsoleListener.h"


class ofApp : public ofBaseApp, public SSHKeyListener{

	public:

		void setup();
		void update();
		void draw();
		
		void keyPressed(int key);
		
		ofShader shader;
		ofFbo fbo;
	
		ofxRaspicam cameraController;
        ConsoleListener consoleListener;
        void onCharacterReceived(SSHKeyListenerEventData& e);
	
};

