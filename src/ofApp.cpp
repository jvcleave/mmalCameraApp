#include "ofApp.h"

ofImage image;
void ofApp::onCharacterReceived(SSHKeyListenerEventData& e)
{
	keyPressed((int)e.character);
}
//--------------------------------------------------------------
void ofApp::setup(){
	//ofSetLogLevel(OF_LOG_VERBOSE); set in main.cpp for core troubleshooting
	consoleListener.setup(this);
	consoleListener.startThread(false, false);
	cameraController.setup();

}

//--------------------------------------------------------------
void ofApp::update(){
	
}

//--------------------------------------------------------------
void ofApp::draw(){
	

}

//--------------------------------------------------------------
void ofApp::keyPressed  (int key){

	ofLogVerbose() << "keyPressed: " << key;
	if (key == 'e') 
	{
		ofLogVerbose() << "e pressed!";
		cameraController.takePhoto();
	}
}