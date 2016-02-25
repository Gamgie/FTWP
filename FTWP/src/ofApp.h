#pragma once

#include "ofMain.h"
#include "ofxKinectForWindows2\src\ofxKinectForWindows2.h"
#include "Spout.h"
#include "ofxOscReceiver.h"
#include "ofxOscSender.h"
#include "ofxGui.h"

class ofApp : public ofBaseApp{

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
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
		void exit();

		ofxKFW2::Device kinect;
		ICoordinateMapper * coordMapper;
	
		vector<ofVec3f> worldCoords;

		Spout spout;
		ofFbo fbo;

		ofxOscSender oscSender;
		ofxOscReceiver oscReceiver;

		//gui and params
		ofxPanel gui;

		ofParameter<string> targetHost;
		ofxButton setHostBT;
		ofParameter<int> targetPort;
		ofxButton setPortBT;
		void setHostPressed();
		void setPortPressed();

		ofParameter<int> sendRate; //fps

		ofParameter<int> resolutionSteps; //fps

		ofParameter<float> depthTolerance;
		ofParameter<float> toleranceDecay;
		ofParameter<float> distanceDecay;
		ofParameter<float> followSpeed;
		ofParameter<int> trackingWidth; 
		ofParameter<int> trackingHeight; 
		ofParameter<float> minPointRatioForAutoTrack; 

		void setKinectConnected(bool value, bool force = false);
};
