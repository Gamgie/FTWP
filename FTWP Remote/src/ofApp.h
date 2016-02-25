#pragma once

#include "ofMain.h"
#include "Spout.h"
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


		SpoutReceiver spout;
		ofxOscSender sender;
		ofImage spoutImage;
		ofPixels spoutPix;


		//gui
		ofxPanel gui;

		ofParameter<string> targetHost;
		ofxButton setHostBT;
		void setHostPressed();

		ofParameter<int> trackingWidth;
		ofParameter<int> trackingHeight;

		ofParameter<float> depthTolerance;
		ofParameter<float> toleranceDecay;
		ofParameter<float> distanceDecay;
		ofParameter<float> followSpeed;
		
		ofParameter<float> minPointRatioForAutoTrack;

		void trackingSizeChanged(int &value);
		void depthToleranceChanged(float &value);
		void toleranceDecayChanged(float &value);
		void distanceDecayChanged(float &value);
		void followSpeedChanged(float &value);
		void minPointRatioChanged(float &value);


};
