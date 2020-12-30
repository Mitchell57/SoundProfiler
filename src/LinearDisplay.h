//
//  LinearDisplay.h
//  SoundProfiler
//
//  Created by Mitch on 12/29/20.
//
#ifndef LinearDisplay_h
#define LinearDisplay_h

#include "Display.h"

class LinearDisplay : public Display {
    
public:
    LinearDisplay();
    
    
    void setup();
    void draw();
    void update(std::vector<utils::soundData> newData);
    void setDimensions(int w, int h);
    void buildGui(ofxGuiGroup* parent);
    
protected:
    void drawLinOctave(int w, int h);
    void drawLinScale(int w, int h);
    
    std::vector<float> octave;
    std::vector<float> scale;
    
    ofParameter<bool> overtoneToggle;
    ofParameter<bool> colorToggle;
    
    float halfW, halfH, xOffset, yOffset;
    int barWidth, margin, maxHeight, y_offset;
    std::vector<std::string> noteNames = {"A", "A#","B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#"};
    
};

#endif /* LinearDisplay_h */
