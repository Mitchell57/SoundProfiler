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
#include "utils.h"


class DisplayMode {
    
    public:
        DisplayMode();
        void init(int w, int h);
    
        // Mode selection
        
        void setMode(utils::Mode m);
        utils::Mode getMode();
    
        // General control
        void draw(Analysis analysis);
        void updateLayout(int w, int h);
    
        // Raw Variables
        bool fftLinear;
    
        // Osc Variables
        void setColorWidth(int val);
        void setColorShift(int val);
        void setSmooth(float val);

    private:
        // General variables
        utils::Mode mode;
        int width, height;
    
    
        // Graph drawers
        void drawLinOctave(int w, int h, int dataSize, float* data);
        void drawLinScale(int w, int h, int dataSize, float* data);
        void drawFftPlot(int w, int h, int dataSize, float* data);
    
        // Graph(Linear, Raw) variables
        float halfW, halfH, xOffset, yOffset;
        int barWidth, margin, maxHeight, y_offset;
        std::vector<string> noteNames = {"A", "A#","B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#"};
    
    
        // Visual drawers
        void drawPolar(int w, int h, int dataSize, float* data);
        void drawOscillator(int w, int h, int dataSize, float* data, float* data2);
    
        // Visual(Osc, Polar) variables
        ofxBlur blur, blur2;
        float timer;
        float* osc_data1;
        float* osc_data2;
        bool osc_started;
        float smooth, sum;
        float x, y, r;
        float* xVals;
        float* yVals;
        float* rVals;
        int colorWidth, colorShift;
};

#endif /* displayModes_hpp */
