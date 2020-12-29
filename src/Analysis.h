//
//  Analysis.h
//  soundProfiler
//
//  Created by Mitch on 12/27/20.
//
#include "ofxFft.h"
#include "utils.h"


#ifndef Analysis_h
#define Analysis_h

class Analysis
{
    public:
        Analysis();
        void init(int bufSize);
    
        // per-frame operations
        void analyzeFrame(std::vector<float> sample, int bufferSize);
        bool smoothFrame();
    
        // getters
        bool isFrameReady();
    
        float* getRawOctave();
        float* getOctave();
        int getOctaveSize();
    
        float* getRawScale();
        float* getScale();
        int getScaleSize();
    
        float* getFft();
        int getFftSize();
    
        float* getData(utils::soundType type);
        int getSize(utils::soundType type);
    
        // setters
        void setAddOvertone(bool b);
        
        
    private:
        ofxFft* fft;
    
        bool frameReady, addOvertone;
        
        int bufferSize, fft_size , oct_size, scale_size;
        
        float* raw_fft;
        float* raw_octave;
        float* smooth_octave;
        float* raw_scale;
        float* smooth_scale;
        float* smooth_scale_ot;

    
        std::vector<int> fullBinList;
        std::vector<float> freqlist;
    
        // constants
        const float a4 = 440;
        std::vector<float> chromaticScale = {440, 466.16, 493.88, 523.25, 554.37, 587.33, 622.25, 659.26, 698.46, 739.99, 783.99, 830.61};
    
};

#endif /* Analysis_h */
