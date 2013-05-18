#pragma once

#include "ofMain.h"
#include "ofxRaspicam.h"
#include "ConsoleListener.h"


class testApp : public ofBaseApp, public SSHKeyListener{

	public:

		void setup();
		void update();
		void draw();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);		
		
		ofShader shader;
		ofFbo fbo;
	
		ofxRaspicam cameraController;
	ConsoleListener consoleListener;
	void onCharacterReceived(SSHKeyListenerEventData& e);
	
};

