//
//  analysis.cpp
//  soundProfiler
//
//  Created by Mitch on 12/27/20.
//
#include "Analysis.h"

// Helper Functions





Analysis::Analysis(){}


//--------------------------------------------------------------
void Analysis::init(int bufSize){
    bufferSize = bufSize;
    fft = ofxFft::create(bufferSize, OF_FFT_WINDOW_BARTLETT);
    
    
    addOvertone = false;
    
    // Data array initialization
    // Builds list of frequencies
    for(int i=-2; i<=3; i++){
        for(int j=0; j<chromaticScale.size(); j++){
            freqlist.push_back(chromaticScale[j]*pow(2, i));
        }
    }
    
    // Builds corresponding list of FFT bins
    // Translate frequency list to bin list
    for(int i=0; i<freqlist.size(); i++){
        float bin = fft->getBinFromFrequency(freqlist[i], 44100);
        fullBinList.push_back((int)bin);
        
    }
    
    // Resize data structures
    // In order to simplify passing octave + individual note data between
    // audio and drawing threads, we use one array where:
    //     audioData[0,oct_size-1] = octave data
    //     audioData[oct_size, wholeDataSize] = individual note data
    oct_size = chromaticScale.size();
    scale_size = fullBinList.size();
    
    
    fft_size = fft->getBinSize();
    
    for(int i=0; i<fft_size; i++){
        in_fft.push_back(0.001);
        raw_fft.push_back(0.001);
    }
    
    
    for(int i=0; i<oct_size; i++){
        raw_octave.push_back(0.001);
        smooth_octave.push_back(0.001);
        
        in_octave.push_back(0.001);
    }
    
    for(int i=0; i<scale_size; i++){
        raw_scale.push_back(0.001);
        smooth_scale.push_back(0.001);
        smooth_scale_ot.push_back(0.001);
        
    }
    
  //  chrom = new Chromagram(Chromagram::Parameters(44100));
    
    
}
//--------------------------------------------------------------
void Analysis::analyzeFrame(std::vector<float> sample, int bufferSize){
    if(!sendToFft){
        analyzeFrameFft(sample, bufferSize);
    }
    else analyzeFrameQ(sample, bufferSize);
}

//--------------------------------------------------------------
void Analysis::analyzeFrameQ(std::vector<float> sample, int bufferSize){
//    CQBase::RealSequence qIn;
//    for(float val : sample){
//        qIn.push_back((double)val);
//    }
//
//    CQBase::RealBlock ret = chrom->process(qIn);
//
//    cout << ret.size() << endl;
    
}



//--------------------------------------------------------------
void Analysis::analyzeFrameFft(std::vector<float> sample, int bufferSize)
{
    frameReady = false;
    
    // Scale audio input frame to {-1, 1}
    float maxValue = 0;
    float* normalizedOut = new float[bufferSize];

    
    for(int i = 0; i < bufferSize; i+=2) {
        if(abs((float)sample[i]) > maxValue) {
            maxValue = abs(sample[i]);
        }
    }
    
    for(int i = 0; i < bufferSize; i++) {
        normalizedOut[i] = (float)sample[i] / maxValue;
    }
    
    // Send scaled frame to FFT
    fft->setSignal(normalizedOut);
    
    delete[] normalizedOut;
    
    // Retrieve analyzed frame
    in_fft = {fft->getAmplitude(), fft->getAmplitude()+fft_size};
    
    float fft_max = 0;
    for(int i=0; i<fft_size; i++){
        if(in_fft[i] > fft_max) fft_max = in_fft[i];
    }

    
    // Max values for normalization
    float scale_max = 0;
    float octave_max = 0;
    
    // Clear out summed data
    for(int i=0; i<oct_size; i++){
        raw_octave[i] = 0;
    }
    

    
    // Record new amplitudes for individual notes and summed notes
    int note;
    float val;
    for(int i=0; i<scale_size; i++){
        // Simplification for summing notes across octaves (i.e. every A, B, C, etc.)
        note = i%12;
        
        val = in_fft[fullBinList[i]];
        
        // record single note data / max
        raw_scale[i] = val; // individual notes start at audioData[oct_size]
        if(val > scale_max) {
            scale_max = val;
        }
        
        // Sum each note across octaves
        raw_octave[note] += val;
    }
    
    // Normalize summed data
    for(int i=0; i<oct_size; i++){
        if(raw_octave[i] > octave_max) octave_max = raw_octave[i];
    }
    
    if(octave_max != 0){
        for(int i=0; i<oct_size; i++){
            raw_octave[i] /= octave_max;
        }
    }
    
    if(scale_max != 0){
        for(int i=0; i<scale_size; i++){
            raw_scale[i] /= scale_max;
        }
    }
//    if(fft_max != 0 && fft_max == fft_max){
//        for(int i=0; i<fft_size; i++){
//            in_fft[i] /= fft_max;
//        }
//    }
    frameReady = smoothFrame();
    
}


//--------------------------------------------------------------
bool Analysis::smoothFrame(){
    // Smoothing+Moving data from buffer
    
    // NaN checker (doesn't get used often but sometimes it's helpful
    if(raw_scale[0] != raw_scale[0]){
        // NaN. skip frame
        
        
        // Gradual fade out for smoothed data sources
        for(float val : smooth_scale){
            val = utils::approxRollingAverage(val, 0, 30);
        }
        
        for(float val : smooth_scale_ot){
            val = utils::approxRollingAverage(val, 0, 30);
        }
        
        for(float val : smooth_octave){
            val = utils::approxRollingAverage(val, 0, 30);
        }
        
        // Clear graphs if NaN and on output mode (usually means file not playing)
        return false;
        // Sometimes buffer will be NaN during input lag
        // So if we reset the graph each time (on input mode) it would be jumpy
        // Haven't observed the same issue on output
    }
    
    raw_fft = in_fft;
    
    // At the moment, smoothing consists of:
    //   - rolling average to make it less 'jumpy'
    for(int i=0; i<oct_size; i++){
        //raw_octave[i] = ofClamp(raw_octave[i], 0, 1); // clamp new data
        smooth_octave[i] = utils::approxRollingAverage(smooth_octave[i], raw_octave[i], 3);
        if(smooth_octave[i] < 0.3) smooth_octave[i] *= smooth_octave[i];
    }
    
    float newVal, overtone;
    for(int i=0; i<scale_size; i++){
        newVal = raw_scale[i];
        overtone = 0;
        int count = 0;
        for(int j=i+12; j<scale_size; j+=12){
            overtone += raw_scale[j];
            count += 1;
        }
        if(count > 0) {
            overtone /= count;
            newVal = (raw_scale[i]+overtone)/2;
        }
        
        smooth_scale_ot[i] = utils::approxRollingAverage(smooth_scale[i], newVal, 3);
        smooth_scale[i] = utils::approxRollingAverage(smooth_scale[i], raw_scale[i], 3);
    }
    
    return true;
}


//---------------------------------------------------------------------------
// getters & setters
//---------------------------------------------------------------------------

//--------------------------------------------------------------
bool Analysis::isFrameReady(){ return frameReady; }




//--------------------------------------------------------------
std::vector<float> Analysis::getData(utils::soundType st){
    switch (st) {
        default: case utils::RAW_FULL:
            return raw_fft;
            break;
        case utils::RAW_OCTAVE:
            return raw_octave;
            break;
        case utils::RAW_SCALE:
            return raw_scale;
            break;
        case utils::SMOOTH_SCALE:
            return smooth_scale;
            break;
        case utils::SMOOTH_OCTAVE:
            return smooth_octave;
            break;
        case utils::SMOOTH_SCALE_OT:
            return smooth_scale_ot;
            break;
    }
}

int Analysis::getSize(utils::soundType st){
    switch (st) {
        default: case utils::RAW_FULL:
            return fft_size;
            break;
        case utils::SMOOTH_OCTAVE:
        case utils::RAW_OCTAVE:
            return oct_size;
            break;
        case utils::RAW_SCALE:
        case utils::SMOOTH_SCALE:
        case utils::SMOOTH_SCALE_OT:
            return scale_size;
            break;
    }
}
