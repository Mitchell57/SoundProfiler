//
//  displayModes.hpp
//  soundProfiler
//
//  Created by Mitch on 12/27/20.
//

#ifndef displayModes_hpp
#define displayModes_hpp

#include <stdio.h>
#include "Analysis.h"
#include "ofxBlur.h"

class DisplayMode {
    
    public:
        DisplayMode();
        void init(int w, int h);
        enum Mode{LINEAR, POLAR, RAW, OSC};
        void setMode(Mode m);
        Mode getMode();
        void draw(Analysis analysis);
        void updateLayout(int w, int h);
    
        bool fftLinear;
    
    private:
        Mode mode;
        int width, height;
        int singleW, singleH, multiW, multiH;
        int singleXOffset, singleYOffset, multiXOffset, multiYOffset;
        int barWidth, margin, maxHeight, y_offset;
    
        ofxBlur blur, blur2;
        ofTime time;
        float timer;
        
        void drawLinOctave(int w, int h, int dataSize, float* data);
        void drawLinScale(int w, int h, int dataSize, float* data);
        void drawPolar(int w, int h, int dataSize, float* data);
        void drawFftPlot(int w, int h, int dataSize, float* data);
        
        void drawOscillator(int w, int h, int dataSize, float* data, float* data2);
        float* osc_data1;
        float* osc_data2;
        bool osc_started;
        float smooth, sum;
        float x, y, r;
        float* xVals;
        float* yVals;
        float* rVals;
    
        std::vector<string> noteNames = {"A", "A#","B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#"};
    
        
    
};








#endif /* displayModes_hpp */
