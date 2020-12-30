//
//  OscDisplay.h
//  SoundProfiler
//
//  Created by Mitch on 12/29/20.
//

#ifndef OscDisplay_h
#define OscDisplay_h

#include "Display.h"
#include "ofxBlur.h"

class OscDisplay : public Display {
    
public:
    OscDisplay();
    
    
    void setup();
    void draw();
    void update(std::vector<utils::soundData> newData);
    void setDimensions(int w, int h);
    void buildGui(ofxGuiGroup *parent);
    
protected:

    std::vector<float> scale;
    std::vector<float> raw_scale;
    
    ofParameter<int> colorWidth;
    ofParameter<int> colorShift;
    ofParameter<float> smooth;
    ofParameter<float> speed;
    
    ofParameter<int> oscColorShift;
    
    ofxGuiGroup* globalGroup;
    ofxGuiGroup* polarGroup;
    ofxGuiGroup* oscGroup;
    
    // Visual drawers
    void drawPolar(int w, int h);
    void drawOscillator(float w, float h);

    // Visual(Osc, Polar) variables
    ofxBlur blur, blur2;
    float timer;
    float sum;
    float x, y, r;
    int dataSize;
    std::vector<float> xVals;
    std::vector<float> yVals;
    std::vector<float> rVals;
    
};

#endif /* OscDisplay_h */
