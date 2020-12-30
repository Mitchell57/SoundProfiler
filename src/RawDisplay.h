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
    void buildGui(ofxGuiGroup* parent);
    
protected:
    void drawFftPlot(int w, int h);
    
    std::vector<float> raw_fft;
    
    float halfW, halfH, xOffset, yOffset;
    float barWidth, margin, maxHeight, y_offset;
    int labelGap, numLabels;
    
    ofxGuiGroup *linLogToggles;
    ofParameterGroup linLogParameters;
    ofParameter<bool> lin, log;
    ofParameter<int> freqsliderStart, freqsliderEnd;
    
    ofParameter<float> freqCenter;
    ofParameter<float> freqWidth;
    ofParameter<void> reset;
    
    void resetParameters();


    void setRawLinLog(int& index);
};

#endif /* RawDisplay_h */
