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

class DisplayMode {
    
    public:
        DisplayMode();
        void init(int w, int h);
        enum Mode{LINEAR, POLAR, RAW, OSC};
        void setMode(Mode m);
        Mode getMode();
        void draw(Analysis analysis);
        void updateLayout(int w, int h);
    
    private:
        Mode mode;
        int width, height;
        int singleW, singleH, multiW, multiH;
        int singleXOffset, singleYOffset, multiXOffset, multiYOffset;
        int barWidth, margin, maxHeight, y_offset;
    
        void drawLinOctave(int w, int h, int dataSize, float* data);
        void drawLinScale(int w, int h, int dataSize, float* data);
        void drawPolar(int w, int h, int dataSize, float* data);
        void drawFftPlot(int w, int h, int dataSize, float* data);
    
        std::vector<string> noteNames = {"A", "A#","B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#"};
    
        
    
};








#endif /* displayModes_hpp */
