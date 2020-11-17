#pragma once

#include "ofMain.h"
#include "ofxFft.h"
#include "ofxStk.h"
#include "ofxGuiExtended.h"
#include <mutex>          // std::mutex

#define WIN_WIDTH 800
#define WIN_HEIGHT 600


class ofApp : public ofBaseApp{

	public:
		
    // openFrameworks
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
        
    
    // audio
        void audioIn(ofSoundBuffer& buffer);
        void audioOut(ofSoundBuffer& buffer);
        void analyzeAudio(std::vector<float> sample, int bufferSize);
        void soundstream_init();
        std::vector<int> freq2bin();
    
        int bufferSize;
        ofxFft* fft;
        stk::FileLoop file;
        ofSoundStream soundStream;
        bool shouldPlayAudio, inputBool, shouldFactorAgg, shouldSmooth;
        
        std::vector<int> fullBinList;
        std::vector<float> freqlist;
        std::vector<float> chromaticScale = {440, 466.16, 493.88, 523.25, 554.37, 587.33, 622.25, 659.26, 698.46, 739.99, 783.99, 830.61};
        std::vector<string> noteNames = {"A", "A#","B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#"};
        std::vector<ofColor> colors = {ofColor::red, ofColor::orange, ofColor::yellow, ofColor::greenYellow, ofColor::green, ofColor::aquamarine, ofColor::cyan, ofColor::cadetBlue, ofColor::blueViolet, ofColor::lavender, ofColor::purple, ofColor::pink};
    
    
    // Drawing
        int barWidth, margin, maxHeight, y_offset;
        void drawSingleOctave(float width, float height);
        void drawMultiOctave(float width, float height);
        void clearGraphs();
    
        //Data
        float* audioData;
        float* buffer;
        float* displayData;
        int wholeDataSize, oct_size, scale_size;
    
        std::mutex mtx;
    
    
    // GUI
        ofxGuiGroup* panel;
        ofxGui gui;
        ofParameterGroup modeGroup;
        ofParameter<bool> inputToggle;
        ofParameter<bool> outputToggle;
        ofParameter<bool> factorToggle;
        ofParameter<bool> smoothToggle;
        ofParameter<void> loadButton;
        ofParameter<void> playButton;
        ofParameter<void> resetButton;
        ofParameter<string> filePath;
    
        ofxGuiGroup *audioModes, *graphControls;
        void inputPressed(bool &inputToggle);
        void outputPressed(bool &outputToggle);
        void factorAggPressed(bool &factorToggle);
        void smoothPressed(bool &smoothToggle);
        void loadFile();
        void loadButtonPresed();
        void playButtonPressed();
        void updateLayout(int w, int h);
    
        int singleW, singleH, multiW, multiH, singleXOffset, singleYOffset, multiXOffset, multiYOffset;
};
