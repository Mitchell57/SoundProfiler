//
//  Analysis.h
//  soundProfiler
//
//  Created by Mitch on 12/27/20.
//
#include "ofxFft.h"


#ifndef Analysis_h
#define Analysis_h

class Analysis
{
    public:
        Analysis();
        void init(int bufSize);
        
        void analyzeFrame(std::vector<float> sample, int bufferSize);
        bool smoothFrame();
    
        bool isFrameReady();
    
        void setAddOvertone(bool b);
    
        float* getRawOctave();
        float* getRawScale();
    
        float* getOctave();
        float* getScale();
    
        int getOctaveSize();
        int getScaleSize();
        
    private:
        ofxFft* fft;
    
        bool frameReady, addOvertone;
        
        int bufferSize;
        int oct_size;
        int scale_size;
    
        float* raw_octave;
        float* raw_scale;
    
        float* smooth_octave;
        float* smooth_scale;

    
        std::vector<int> fullBinList;
        std::vector<float> freqlist;
    
        // constants
        const float a4 = 440;
        std::vector<float> chromaticScale = {440, 466.16, 493.88, 523.25, 554.37, 587.33, 622.25, 659.26, 698.46, 739.99, 783.99, 830.61};
    
};





#endif /* Analysis_h */
