#include "ofApp.h"


#define IMAGE_WIDTH 512
#define IMAGE_HEIGHT 424
#define NUM_PIXELS IMAGE_WIDTH*IMAGE_HEIGHT

int panelWidth = 200;

unsigned int spoutTexWidth = 0;
unsigned int spoutTexHeight = 0;

char senderName[256] = "FTWP";

ofPoint sourcePoint;

int targetPort = 9081;

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetFrameRate(30);

	ofSetWindowShape(IMAGE_WIDTH + panelWidth, IMAGE_HEIGHT);
	
	spoutPix.allocate(IMAGE_WIDTH, IMAGE_HEIGHT, OF_IMAGE_COLOR_ALPHA);

	spout.CreateReceiver(senderName, spoutTexWidth, spoutTexHeight);
	
	sender.setup("127.0.0.1", 9081);

	sourcePoint.set(0, 0);


	gui.setup("FTWP Remote", "settings.xml", IMAGE_WIDTH, 10); // most of the time you don't need a name but don't forget to call setup

	gui.add(setHostBT.setup("Set Host"));
	gui.add(targetHost.set("Target Host", targetHost));
	setHostBT.addListener(this, &ofApp::setHostPressed);


	gui.add(trackingWidth.set("Tracking Width", trackingWidth, 10, 400));
	gui.add(trackingHeight.set("Tracking Height", trackingHeight, 10, 400));
	gui.add(depthTolerance.set("Depth Tolerance", depthTolerance, 0, 0.2));
	gui.add(toleranceDecay.set("Tolerance Decay", toleranceDecay, .1, 2));
	gui.add(distanceDecay.set("Distance Decay", distanceDecay, .1, 4));
	gui.add(followSpeed.set("Follow Speed", followSpeed, 0, 10));
	gui.add(minPointRatioForAutoTrack.set("Min Points Ratio", minPointRatioForAutoTrack));


	trackingWidth.addListener(this, &ofApp::trackingSizeChanged);
	trackingHeight.addListener(this, &ofApp::trackingSizeChanged);
	depthTolerance.addListener(this, &ofApp::depthToleranceChanged);
	toleranceDecay.addListener(this, &ofApp::toleranceDecayChanged);
	distanceDecay.addListener(this, &ofApp::distanceDecayChanged);
	followSpeed.addListener(this, &ofApp::followSpeedChanged);
	minPointRatioForAutoTrack.addListener(this, &ofApp::minPointRatioChanged);

	gui.loadFromFile("settings.xml");
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
	ofBackground(0);

	
	spout.ReceiveImage(senderName, spoutTexWidth, spoutTexHeight,spoutPix);
	spoutImage.setFromPixels(spoutPix);

	spoutImage.draw(0, 0);

	gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{
	ofxOscMessage msg;

	switch (button)
	{
	case OF_MOUSE_BUTTON_LEFT:
		sourcePoint.set(x, y);
		msg.setAddress("/ftwp/source");
		msg.addInt32Arg(x);
		msg.addInt32Arg(y);
		sender.sendMessage(msg);
		break;

	case OF_MOUSE_BUTTON_RIGHT:
		msg.setAddress("/ftwp/tolerance");
		msg.addFloatArg(x*0.1/ IMAGE_WIDTH);
		sender.sendMessage(msg);
		break;

	case OF_MOUSE_BUTTON_MIDDLE:
		msg.setAddress("/ftwp/trackinSize");
		trackingWidth.set(abs(x - sourcePoint.x));
		trackingHeight.set(abs(x - sourcePoint.x));
		msg.addInt32Arg(abs(y - sourcePoint.y));
		sender.sendMessage(msg);
		break;

	}

}

void ofApp::setHostPressed()
{
	string host = ofSystemTextBoxDialog("Target host :", targetHost);
	targetHost.set(host);
	sender.setup(targetHost, targetPort);
};

void ofApp::trackingSizeChanged(int &value)
{
	ofxOscMessage msg;
	msg.setAddress("/ftwp/trackingSize");
	msg.addInt32Arg(trackingWidth);
	msg.addInt32Arg(trackingHeight);
	sender.sendMessage(msg);

}


void ofApp::depthToleranceChanged(float &value)
{
	ofxOscMessage msg;
	msg.setAddress("/ftwp/tolerance");
	msg.addFloatArg(depthTolerance);
	sender.sendMessage(msg);
}

void ofApp::toleranceDecayChanged(float &value)
{
	ofxOscMessage msg;
	msg.setAddress("/ftwp/toleranceDecay");
	msg.addFloatArg(toleranceDecay);
	sender.sendMessage(msg);
}

void ofApp::distanceDecayChanged(float &value)
{
	ofxOscMessage msg;
	msg.setAddress("/ftwp/distanceDecay");
	msg.addFloatArg(distanceDecay);
	sender.sendMessage(msg);
}
void ofApp::followSpeedChanged(float &value)
{
	ofxOscMessage msg;
	msg.setAddress("/ftwp/followSpeed");
	msg.addFloatArg(followSpeed);
	sender.sendMessage(msg);
}

void ofApp::minPointRatioChanged(float &value)
{
	ofxOscMessage msg;
	msg.setAddress("/ftwp/minPointRatio");
	msg.addFloatArg(minPointRatioForAutoTrack);
	sender.sendMessage(msg);
}


//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
	
	ofxOscMessage msg;
	switch (button)
	{
	case OF_MOUSE_BUTTON_LEFT:
		
		msg.setAddress("/ftwp/source/release");
		sender.sendMessage(msg);
		break;

	case OF_MOUSE_BUTTON_RIGHT:
		bool result = spout.SelectSenderPanel();
		printf("Check spout panel : %i\n", result);
		break;
	}
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

void ofApp::exit()
{
	gui.saveToFile("settings.xml");
}