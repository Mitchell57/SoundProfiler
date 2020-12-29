//
//  LinearDisplay.h
//  SoundProfiler
//
//  Created by Mitch on 12/29/20.
//
#include "Display.h"
#include "ofxGuiExtended.h"

#ifndef LinearDisplay_h
#define LinearDisplay_h

class LinearDisplay : public Display {
    
public:
    LinearDisplay();
    
    void setup();
    void draw();
    void update();
    void setDimensions(int w, int h);
    
protected:
    ofParameter<bool> factorToggle;
    
    float halfW, halfH, xOffset, yOffset;
    int barWidth, margin, maxHeight, y_offset;
    std::vector<std::string> noteNames = {"A", "A#","B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#"};
    
};

#endif /* LinearDisplay_h */
