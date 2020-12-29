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
    void setup(Analysis* a, int w, int h);
    
    // general control
    void draw();
    void update();
    void updateLayout(int w, int h);
    
    // mode selection
    void setMode(int index);
    int getMode();
    
    ofParameterGroup parameters;
    
    ofxGuiGroup *modeSelectorGroup; // add mode buttons
    
    ofxGuiGroup *modeControlGroup; // add all mode-specific parameters, only show current mode
    
    
protected:
    std::vector<Display*> modes;
    
    
    bool ready{};
    int width, height;
    int current_mode;
    Analysis* analysis;
    
};


#endif /* DisplayController_h */
