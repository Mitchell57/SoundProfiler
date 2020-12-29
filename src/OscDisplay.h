//
//  OscDisplay.h
//  SoundProfiler
//
//  Created by Mitch on 12/29/20.
//

#ifndef OscDisplay_h
#define OscDisplay_h

#include "Display.h"
#include "ofxGuiExtended.h"
#include "ofxBlur.h"
#include "utils.h"

class OscDisplay : public Display {
    
public:
    OscDisplay();
    
    
    void setup();
    void draw();
    void update(std::vector<utils::soundData> newData);
    void setDimensions(int w, int h);
    
protected:

    std::vector<float> scale;
    std::vector<float> raw_scale;
    
    ofParameter<int> colorWidth;
    ofParameter<int> colorShift;
    ofParameter<float> smooth;
    
    // Visual drawers
    void drawPolar(int w, int h);
    void drawOscillator(int w, int h);

    // Visual(Osc, Polar) variables
    ofxBlur blur, blur2;
    float timer;
    float* osc_data1;
    float* osc_data2;
    bool osc_started;
    float sum;
    float x, y, r;
    float* xVals;
    float* yVals;
    float* rVals;
    
};

#endif /* OscDisplay_h */
