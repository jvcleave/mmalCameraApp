#include "testApp.h"

ofImage image;
void testApp::onCharacterReceived(SSHKeyListenerEventData& e)
{
	keyPressed((int)e.character);
}
//--------------------------------------------------------------
void testApp::setup(){
	//ofSetLogLevel(OF_LOG_VERBOSE); set in main.cpp for core troubleshooting
	consoleListener.setup(this);
	consoleListener.startThread(false, false);
	cameraController.setup();

}

//--------------------------------------------------------------
void testApp::update(){
	
}

//--------------------------------------------------------------
void testApp::draw(){
	

}

//--------------------------------------------------------------
void testApp::keyPressed  (int key){

	ofLogVerbose() << "keyPressed: " << key;
	if (key == 'e') 
	{
		ofLogVerbose() << "e pressed!";
		cameraController.takePhoto();
	}
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){


}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}


//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}

