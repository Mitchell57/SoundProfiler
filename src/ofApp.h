#pragma once

#include "ofMain.h"
#include "ofxFft.h"
#include "ofxStk.h"
#include "ofxGuiExtended.h"
#include "Analysis.h"
#include "DisplayMode.hpp"
#include <mutex>          // std::mutex

#define WIN_WIDTH 1000
#define WIN_HEIGHT 800


class ofApp : public ofBaseApp{

	public:
		
    // openFrameworks builtins
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

        void soundstream_init();
    
    
        Analysis analysis;
    
        int bufferSize;
        stk::FileLoop file;
        ofSoundStream soundStream;
        bool shouldPlayAudio, shouldFactorAgg, shouldSmooth;
        bool inputBool, loadPressed, playPressed;
        
        
    
    
    // Drawing
        DisplayMode dm;
        void clearGraphs();
    

    
    
    // GUI
        ofxGuiGroup* all;
        ofxGui gui;
        ofParameterGroup modeGroup;

        ofParameter<bool> inputToggle;
        ofParameter<bool> outputToggle;
        ofParameter<bool> factorToggle;
        ofParameter<bool> smoothToggle;
        ofParameter<void> loadButton;
        ofParameter<void> playButton;
        ofParameter<void> resetButton;
        ofParameter<void> closeButton;
        ofParameter<string> filePath;
    
        // Display Mode Controles
        ofxGuiGroup *displayToggles;
        ofParameterGroup displayParameters;
        ofParameter<bool> disp0;
        ofParameter<bool> disp1;
        ofParameter<bool> disp2;
        ofParameter<bool> disp3;
    
        void setDisplayMode(int& index);
    
        ofxGuiGroup *inputToggles;
        ofParameterGroup inputParameters;
        ofParameter<bool> input0;
        ofParameter<bool> input1;
    
        void setInputMode(int& index);
    
    
    
        ofxGuiGroup *audioModes, *graphControls, *fileManager;
        void inputPressed(bool &inputToggle);
        void outputPressed(bool &outputToggle);
        void factorAggPressed(bool &factorToggle);
        void smoothPressed(bool &smoothToggle);
        void closeMenu();
        void loadFile();
        void playFile();
        void updateLayout(int w, int h);
    
    
    
    
        int viewMode;
        int width, height, controlWidth;
};
