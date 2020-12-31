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
#include <mutex>


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
    
    void drawLinearFftWindow(float w, float h);
    
    std::vector<float> raw_fft;
    std::vector<float> fft_display;
    std::vector<float> fft_display_freqs;
    
    float halfW, halfH, xOffset, yOffset;
    float barWidth, margin, maxHeight, y_offset;
    int labelGap, numLabels;
    
    ofxGuiGroup *linLogToggles, *windowGroup;
    ofParameterGroup linLogParameters;
    ofParameter<bool> lin, log;
    ofParameter<bool> rescale;
    ofParameter<int> freqsliderStart, freqsliderEnd;
    
    ofParameter<float> freqCenter;
    ofParameter<float> freqWidth;
    
    ofParameter<float> smooth;
    ofParameter<void> reset;
    
    int startBin, endBin;
    
    void resetParameters();
    void fftWindowChanged(float& val);
    float size;

    void setRawLinLog(int& index);
    
    std::mutex mtx;
};

#endif /* RawDisplay_h */
