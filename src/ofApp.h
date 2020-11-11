#pragma once

#include "ofMain.h"
#include "ofxFft.h"
#include "chromaticParse.hpp"
#include <mutex>          // std::mutex

#define WIN_WIDTH 600
#define WIN_HEIGHT 600

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
        void exit();
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
        
        void audioReceived(float* input, int bufferSize, int nChannels);
        std::vector<int> freq2bin();

        int bufferSize;
    
        ofxFft* fft;
        ofSoundPlayer player;

        std::vector<float> chromaticScale = {440, 466.16, 493.88, 523.25, 554.37, 587.33, 622.25, 659.26, 698.46, 739.99, 783.99, 830.61};
        std::vector<string> noteNames = {"A", "A#","B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#"};
        std::vector<ofColor> colors = {ofColor::red, ofColor::orange, ofColor::yellow, ofColor::greenYellow, ofColor::green, ofColor::aquamarine, ofColor::cyan, ofColor::cadetBlue, ofColor::blueViolet, ofColor::lavender, ofColor::purple, ofColor::pink};
    
        int barWidth, margin, maxHeight, y_offset;
        float* singleOctave;
        float* buffer;
        float* displayOctave;
        int oct_size;
    
        std::mutex mtx;
};
