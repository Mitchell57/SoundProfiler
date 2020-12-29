//
//  DisplayController.h
//  SoundProfiler
//
//  Created by Mitch on 12/29/20.
//
#include "utils.h"
#include "Analysis.h"
#include "ofxGuiExtended.h"
#include "Display.h"
#include "LinearDisplay.h"

#ifndef DisplayController_h
#define DisplayController_h

class DisplayController{
    
public:
    DisplayController();
    
    // setup
    void setup(Analysis* a, int w, int h, ofxGuiGroup* all);
    
    // general control
    void draw();
    void update();
    void updateLayout(int w, int h);
    void minimize();
    
    // mode selection
    void setMode(int index);
    int getMode();
    
    ofParameterGroup modeSelector;
    
    ofxGuiGroup *modeSelectorGroup; // add mode buttons
    
    ofxGuiGroup *modeControlGroup; // add all mode-specific parameters, only show current mode
    
    
protected:
    std::vector<Display*> modes;
    std::vector<ofxGuiGroup*> modeControls;
    
    ofParameter<bool> disp0, disp1, disp2;
    
    void setDisplayMode(int& index);
    
    
    bool ready{};
    int width, height;
    int current_mode;
    Analysis* analysis;
    
};


#endif /* DisplayController_h */
