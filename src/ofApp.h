#pragma once

#include "ofMain.h"
#include "ofxFft.h"
#include "ofxStk.h"
#include "ofxGuiExtended.h"
#include "Analysis.h"
#include "DisplayMode.hpp"
#include "utils.h"
#include "DisplayController.h"

#define WIN_WIDTH 1000
#define WIN_HEIGHT 800


class ofApp : public ofBaseApp{

	public:
            
        //--------------------------------------------------------------------------------
        // builtins
        //--------------------------------------------------------------------------------
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
        
        
        //--------------------------------------------------------------------------------
        // audio
        //--------------------------------------------------------------------------------
        void audioIn(ofSoundBuffer& buffer);
        void audioOut(ofSoundBuffer& buffer);
        void soundstream_init();
    
        Analysis analysis;
    
        int bufferSize;
        stk::FileLoop file;
        ofSoundStream soundStream;
        bool shouldPlayAudio{}, shouldFactorAgg{};
        
        
        //--------------------------------------------------------------------------------
        // drawing
        //--------------------------------------------------------------------------------
        void updateLayout(int w, int h);
        int controlWidth;
    
        DisplayController dc;
    
        //--------------------------------------------------------------------------------
        // GUI
        //--------------------------------------------------------------------------------
    
        //--------------------------------------------------------------------------------
        //   general
        //--------------------------------------------------------------------------------
        ofxGuiGroup* all;
        ofxGui gui;
        ofxGuiGroup* modeControls;
        ofParameter<void> minimizeButton;
    
        void minimizePressed();
    
    

    
    
        //--------------------------------------------------------------------------------
        //   input mode
        //--------------------------------------------------------------------------------
        ofxGuiGroup *inputToggles;
        ofParameterGroup inputParameters;
        ofParameter<bool> input0;
        ofParameter<bool> input1;

        void setInputMode(int& index);
    
    
        //--------------------------------------------------------------------------------
        //   file manager
        //--------------------------------------------------------------------------------
        ofxGuiGroup *fileManager;
        ofxGuiGroup *playbackControls;
        ofParameter<void> loadButton;
        ofParameter<void> playButton;
        ofParameter<void> resetButton;
        ofParameter<string> filePath;
    
        bool inputBool{true}, fileLoaded{};
        bool loadPressed{}, playPressed{}, resetPressed{};
    
        void loadFile();
        void playFile();
        void restartFile();
        
        
        
};
