#pragma once

#include "ofMain.h"
#include "ofxFft.h"
#include "chromaticParse.hpp"

#define WIN_WIDTH 600
#define WIN_HEIGHT 600

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
    
        void chromaticPlot(vector<float> buffer, float width, float height);
        void rawPlot(vector<float> buffer, float width, float height);
        void barkPlot(vector<int> buffer, float width, float height);
        void harmonicPlot(vector<float> buffer, float width, float height);
        void audioReceived(float* input, int bufferSize, int nChannels);
        void chromaticMap(vector<float> buffer, ofImage img);
        
    
        int plotHeight, bufferSize, barkSize, windowSize, width, height;
        float maxUp, maxDown;
        
        ChromaticParse cp;
    
        ofxFft* fft;
        ofImage img;
        ofShader shader;
        ofPlanePrimitive plane;
        
        ofMutex soundMutex;
        std::vector<int> middleBins;
        std::vector<float> deltaBins;
        std::vector<float> superRaw;

        std::vector<string> noteNames = {"A", "A#","B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#"};
        std::vector<ofColor> colors = {ofColor::red, ofColor::orange, ofColor::yellow, ofColor::greenYellow, ofColor::green, ofColor::aquamarine, ofColor::cyan, ofColor::cadetBlue, ofColor::blueViolet, ofColor::lavender, ofColor::purple, ofColor::pink};
    
        int barWidth, margin, maxHeight, y_offset;
    
};
