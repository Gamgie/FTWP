#include "ofApp.h"


#define IMAGE_WIDTH 512
#define IMAGE_HEIGHT 424
#define NUM_PIXELS IMAGE_WIDTH*IMAGE_HEIGHT

int panelWidth = 200;

float lastNewFrameTime = 0;
float maxNoNewFrameTime = .5;
bool kinectIsConnected = false;

//PARAMS - Controlled with ofxGui

//float depthTolerance = .007;
//float minPointRatioForAutoTrack = .2;
//float followSpeed = 8;
//int sendRate = 30; //fps

//Tracking data
ofVec2f sourcePoint; // controlled by mouse and autotrack
float trackingDepth = .5; //controlled by mouse and autotrack
ofPoint lastAveragePoint;
float lastAverageDepth = 0;
bool manualOverride = false;
float pointRatio = 0;


//sending
float lastSendTime = 0;

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetFrameRate(50);
	ofSetWindowShape(IMAGE_WIDTH+panelWidth, IMAGE_HEIGHT);

	kinect.open();
	kinect.initDepthSource();
	kinect.initColorSource();
	kinect.initInfraredSource();

	if (kinect.getSensor()->get_CoordinateMapper(&coordMapper) < 0) {
		printf("Could not acquire CoordinateMapper!");
	}


	//sourcePoint.set(.4, .5);
	//trackingSize.set(.3,.3);

	//Init variables
	lastAveragePoint.set(sourcePoint);
	worldCoords.resize(NUM_PIXELS);


	// ====== SPOUT =====
	spout.CreateSender("FTWP", IMAGE_WIDTH, IMAGE_HEIGHT);
	fbo.allocate(IMAGE_WIDTH, IMAGE_HEIGHT);


	

	//gui
	gui.setup("FTWP", "settings.xml", IMAGE_WIDTH, 10); // most of the time you don't need a name but don't forget to call setup

	gui.add(setHostBT.setup("Set Host"));
	gui.add(setPortBT.setup("Set Port"));
	gui.add(targetHost.set("Target Host", targetHost));
	gui.add(targetPort.set("Target Port", targetPort, 2000, 10000));
	setHostBT.addListener(this, &ofApp::setHostPressed);
	setPortBT.addListener(this, &ofApp::setPortPressed);

	gui.add(sendRate.set("Send Rate", sendRate, 1, 100));
	
	gui.add(resolutionSteps.set("Resolution Steps", resolutionSteps, 1, 20));

	gui.add(trackingWidth.set("Tracking Width", trackingWidth,10,400));
	gui.add(trackingHeight.set("Tracking Height", trackingHeight,10,400));
	
	gui.add(depthTolerance.set("Depth Tolerance",depthTolerance,0,0.2));
	gui.add(toleranceDecay.set("Tolerance Decay", toleranceDecay, .1, 2));
	gui.add(distanceDecay.set("Distance Decay", distanceDecay, .1, 4));
	gui.add(followSpeed.set("Follow Speed", followSpeed,0,10));
	gui.add(minPointRatioForAutoTrack.set("Min Points Ratio", minPointRatioForAutoTrack));

	gui.loadFromFile("settings.xml");

	//OSC
	oscSender.setup(targetHost,targetPort);
	oscReceiver.setup(9081);
}

//--------------------------------------------------------------
void ofApp::update() {

	// check for waiting messages
	while (oscReceiver.hasWaitingMessages()) {
		// get the next message
		ofxOscMessage m;
		oscReceiver.getNextMessage(m);

		// check for mouse moved message
		if (m.getAddress() == "/ftwp/source") {
			sourcePoint.set(m.getArgAsInt(0), m.getArgAsInt(1));
			manualOverride = true;
		}else if (m.getAddress() == "/ftwp/source/release") {
			manualOverride = false;
		}
		else if (m.getAddress() == "/ftwp/steps") {
			// the single argument is a string
			resolutionSteps = m.getArgAsInt(0);
		}
		// check for mouse button message
		else if (m.getAddress() == "/ftwp/tolerance") {
			// the single argument is a string
			depthTolerance = m.getArgAsFloat(0);
		}
		else if (m.getAddress() == "/ftwp/toleranceDecay") {
			// the single argument is a string
			toleranceDecay = m.getArgAsFloat(0);
		}
		else if (m.getAddress() == "/ftwp/distanceDecay") {
			// the single argument is a string
			distanceDecay = m.getArgAsFloat(0);
		}
		// check for an image being sent (note: the size of the image depends greatly on your network buffer sizes - if an image is too big the message won't come through ) 
		else if (m.getAddress() == "/ftwp/trackingSize") {
			trackingWidth.set(m.getArgAsInt(0));
			trackingHeight.set(m.getArgAsInt(1));
		}
		else if (m.getAddress() == "/ftwp/followSpeed") {
			followSpeed.set(m.getArgAsFloat(0));
		}
		else if (m.getAddress() == "/ftwp/minPointRatio") {
			minPointRatioForAutoTrack.set(m.getArgAsFloat(0));
		}
		else if (m.getAddress() == "/k2s/device/connected") {
			// the single argument is a string
			setKinectConnected(kinectIsConnected, true);
		}
	}
	
	kinect.update();

	if (kinect.isFrameNew())
	{
		lastNewFrameTime = ofGetElapsedTimef();
		setKinectConnected(true);
	}
	else
	{
		if (ofGetElapsedTimef() - lastNewFrameTime > maxNoNewFrameTime)
		{
			setKinectConnected(false);
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw() {

	ofBackground(0,255);
	

	if (kinectIsConnected)
	{
		//processDepthImage();

		ofxKinectForWindows2::Source::Depth * depth = kinect.getDepthSource().get();
		int tw = depth->getWidth();
		int th = depth->getHeight();

		int minP = 1000;
		int maxP = 0;

		ofImage depthImage;
		ofImage threshImage;
		threshImage.allocate(IMAGE_WIDTH, IMAGE_HEIGHT, OF_IMAGE_COLOR_ALPHA);
		
		//ofFloatPixels worldPixels = this->kinect.getDepthSource()->getDepthToWorldMap();


		ofShortPixels pixels = this->kinect.getDepthSource()->getPixels();
		//ofPixels threshPixels;
		//threshPixels.allocate(IMAGE_WIDTH, IMAGE_HEIGHT, OF_IMAGE_COLOR_ALPHA);
		//threshImage.setFromPixels(pixels);

		int sourceIndex = sourcePoint.y*IMAGE_WIDTH + sourcePoint.x;
		if (sourceIndex >= 0 && sourceIndex < pixels.size())
		{
			float newSourceDepth = pixels[sourceIndex] / 8192.0;

			if (!manualOverride)
			{
				if (pointRatio >= minPointRatioForAutoTrack)
				{
					sourcePoint.set(sourcePoint + (lastAveragePoint - sourcePoint)*followSpeed*ofGetLastFrameTime());
					trackingDepth = lastAverageDepth;// trackingDepth + (newSourceDepth - trackingDepth) * followSpeedDepth*ofGetLastFrameTime();
				}
			}
			else
			{
				trackingDepth = newSourceDepth;
			}
		}

		lastAveragePoint.set(0, 0);
		float averageWeightSum = 0;
		float averageDistSum = 0;

		int numGoodPoints = 0;

		lastAverageDepth = 0;

		int steps = 4;

		for (int i = 0; i < NUM_PIXELS; i++) {
			
			
			int tx = i%IMAGE_WIDTH;
			int ty = floor(i / IMAGE_WIDTH);
			ofPoint p;
			p.set(tx, ty);

			float depthP = pixels[i] * 1.0 / 8192.0;
			pixels[i] = depthP * 65536;

			if (i % resolutionSteps != 0)
			{
				threshImage.setColor(tx, ty, ofColor(0, 0));
				continue;
			}

			float distToTrackingDepth = abs(depthP - trackingDepth);
			bool isInThresh = distToTrackingDepth < depthTolerance;
			bool isInRect = abs(p.x - sourcePoint.x) < trackingWidth && abs(p.y - sourcePoint.y) < trackingHeight;

			

			unsigned char colP = (unsigned char)(depthP * 255);
			
			if (isInThresh && isInRect && depthP > 0)
			{
				ofColor c1 = ofColor(0, 255, 0, 200);
				ofColor c2 = ofColor(255, 0, 0, 150);
				ofColor c3 = ofColor(0, 0, 255, 150);

				float relDepthDist = min<float>(distToTrackingDepth / (depthTolerance*toleranceDecay),1.0);
				float distToSourcePoint = min<float>(p.distance(sourcePoint) / (((trackingWidth + trackingHeight) / 2)*distanceDecay),1.0);
				
				ofColor c = c1.lerp(c2, relDepthDist);
				c = c.lerp(c3, distToSourcePoint);

				threshImage.setColor(tx,ty, c);

				lastAverageDepth += depthP*(1 - relDepthDist);
				averageWeightSum += 1 - relDepthDist;

	
				lastAveragePoint += p*(1-distToSourcePoint);
				averageDistSum += 1 - distToSourcePoint;
				
				numGoodPoints++;
			}else
			{
				threshImage.setColor(tx,ty, ofColor(0,0));
			}
		}

		threshImage.update();

		lastAveragePoint /= averageDistSum;
		lastAverageDepth /= averageWeightSum;


		pointRatio = numGoodPoints*1.0 / (trackingWidth*trackingHeight);


		ofVec3f worldPoint;
		worldPoint.set(0, 0, 0);

		
		coordMapper->MapDepthFrameToCameraSpace(NUM_PIXELS, (UINT16*) this->kinect.getDepthSource()->getPixels(), NUM_PIXELS,(CameraSpacePoint *)worldCoords.data());

		int pointExpansion = 5;
		int numGoodExpansion = 0;
		for (int i = -pointExpansion; i < pointExpansion; i++)
		{
			for (int j = -pointExpansion; j < pointExpansion;j++)
			{
				int tx = lastAveragePoint.x + j;
				int ty = lastAveragePoint.y + i;
				int index = (ty*IMAGE_WIDTH + tx); //x,y,z in ofFloatPixels
				if (index > 0 && index < worldCoords.size())
				{
					ofVec3f expPoint = worldCoords[index];
					if (expPoint.z > 0)
					{
						worldPoint += expPoint;
						numGoodExpansion++;
					}
				}
			}
		}

		worldPoint /= numGoodExpansion;
		
		fbo.begin();
		ofBackground(0);
		depthImage.setFromPixels(pixels);
		
		ofSetColor(255);
		depthImage.draw(0, 0);
		threshImage.draw(0, 0);

		ofPushStyle();
		ofFill();
		ofSetColor(255, 255, 0);
		ofDrawCircle(sourcePoint, 5);
		ofNoFill();
		ofSetRectMode(OF_RECTMODE_CENTER);
		ofDrawRectangle(sourcePoint, trackingWidth*2, trackingHeight*2);

		ofFill();
		ofSetColor(0, 255, 255);
		ofDrawCircle(lastAveragePoint, 5);
		
		std::stringstream ss;
		ss << ofToString(worldPoint.x,2) << "\n" << ofToString(worldPoint.y,2) << "\n" << ofToString(worldPoint.z,2);
		std::string pString = ss.str();

		ofDrawBitmapString(pString, lastAveragePoint+ofPoint(10,0));

		ofPopStyle();
		fbo.end();
		

		ofPixels pix;
		pix.allocate(IMAGE_WIDTH, IMAGE_HEIGHT, OF_IMAGE_COLOR_ALPHA);
		fbo.readToPixels(pix);

		fbo.draw(0, 0);


		//Sending
		spout.SendImage(pix.getData(), depthImage.getWidth(), depthImage.getHeight(),6408U,true,false);
		int sendTime = 1.0f / sendRate;


		if (ofGetElapsedTimef() - lastSendTime > sendTime)
		{
			ofxOscMessage msg;
			msg.setAddress("/ftwp/position");
			msg.addFloatArg(worldPoint.x);
			msg.addFloatArg(worldPoint.y);
			msg.addFloatArg(worldPoint.z);
			oscSender.sendMessage(msg, false);
			lastSendTime = ofGetElapsedTimef();
		}
	}else
	{
		ofPushMatrix();
		ofPushStyle();
		ofTranslate(50, 50);
		ofScale(3, 3);
		ofSetColor(ofColor::orangeRed);
		ofDrawBitmapString("KINECT IS NOT CONNECTED", 0, 0);
		ofPopStyle();
		ofPopMatrix();
	}
	
	ofDrawBitmapString(ofToString(ofGetFrameRate()) + " FPS", 10, 10);
	gui.draw();
}


void ofApp::setKinectConnected(bool value, bool force)
{
	if (kinectIsConnected == value && !force) return;
	kinectIsConnected = value;

	printf("Kinect Connection Changed : %i\n", kinectIsConnected);
	ofxOscMessage msg;
	msg.setAddress("/ftwp/device/connected");
	msg.addIntArg(kinectIsConnected ? 1 : 0);
	oscSender.sendMessage(msg, false);

}


void ofApp::setHostPressed()
{
	string host = ofSystemTextBoxDialog("Target host :", targetHost);
	targetHost.set(host);
	oscSender.setup(targetHost.get(), targetPort.get());
};


void ofApp::setPortPressed()
{
	string port = ofSystemTextBoxDialog("Target port :", targetPort.toString());
	targetPort.set(ofToInt(port));
	oscSender.setup(targetHost.get(), targetPort.get());
};

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
void ofApp::mouseDragged(int x, int y, int button){
	switch (button)
	{
	case OF_MOUSE_BUTTON_LEFT:
		sourcePoint.set(x, y);
		break;

	case OF_MOUSE_BUTTON_RIGHT:
		//trackingDepth = y*1.0 / ofGetHeight();
		depthTolerance = x*.1 / IMAGE_WIDTH;
		printf("Depth tolerance : %f\n", depthTolerance);
		break;

	case OF_MOUSE_BUTTON_MIDDLE:
		trackingWidth = abs(x - sourcePoint.x);
		trackingHeight = abs(y - sourcePoint.y);
		break;

	}
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	manualOverride = true;
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
	manualOverride = false;
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