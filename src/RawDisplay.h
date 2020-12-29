//
//  RawDisplay.h
//  SoundProfiler
//
//  Created by Mitch on 12/29/20.
//

#ifndef RawDisplay_h
#define RawDisplay_h

#include "Display.h"
#include "ofxGuiExtended.h"

class RawDisplay : public Display {
    
public:
    RawDisplay();
    
    
    void setup();
    void draw();
    void update(std::vector<utils::soundData> newData);
    void setDimensions(int w, int h);
    
protected:
    void drawFftPlot(int w, int h);
    
    std::vector<float> raw_fft;
    
    float halfW, halfH, xOffset, yOffset;
    int barWidth, margin, maxHeight, y_offset;
    
    ofxGuiGroup *linLogToggles;
    ofParameterGroup linLogParameters;
    ofParameter<bool> linlog;


    void setRawLinLog(int& index);
};

#endif /* RawDisplay_h */
