//
//  analysis.cpp
//  soundProfiler
//
//  Created by Mitch on 12/27/20.
//
#include "Analysis.h"

// Helper Functions


// Approximate Rolling Average from:
// https://bit.ly/3aIsQRD
float approxRollingAverage (float avg, float new_sample) {

    avg -= avg / 2;
    avg += new_sample / 2;

    return avg;
}


Analysis::Analysis(){
    // stub
}


//--------------------------------------------------------------
void Analysis::init(int bufSize){
    bufferSize = bufSize;
    fft = ofxFft::create(bufferSize, OF_FFT_WINDOW_BARTLETT);
    
    
    addOvertone = true;
    
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
    raw_fft = new float[fft_size];
    
    raw_octave = new float[oct_size];
    raw_scale = new float[scale_size];
    
    smooth_octave = new float[oct_size];
    smooth_scale = new float[scale_size];
}


//--------------------------------------------------------------
void Analysis::analyzeFrame(std::vector<float> sample, int bufferSize)
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
    
    for(int i=0; i<fft_size; i++){
        raw_fft[i] = fft->getAmplitudeAtBin(i);
    }
    
    // Audio listeners run in a separate thread
    // so we must ensure control before making changes to a shared variable
    //const std::lock_guard<std::mutex> lock(mtx);
    
    // Max values for normalization
    float scale_max = 0;
    float octave_max = 0;
    
    // Clear out summed data
    for(int i=0; i<oct_size; i++){
        raw_octave[i] = 0;
    }
    
    
    // Record new amplitudes for individual notes and summed notes
    int note;
    for(int i=0; i<scale_size; i++){
        // Simplification for summing notes across octaves (i.e. every A, B, C, etc.)
        note = i%12;
        
        float val = fft->getAmplitudeAtBin(fullBinList[i]);
        
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
    
    for(int i=0; i<oct_size; i++){
        raw_octave[i] /= octave_max;
    }
    
    for(int i=0; i<scale_size; i++){
        raw_scale[i] /= scale_max;
    }
    
    frameReady = smoothFrame();
    
}


//--------------------------------------------------------------
bool Analysis::smoothFrame(){
    // Smoothing+Moving data from buffer
    
    // NaN checker (doesn't get used often but sometimes it's helpful
    if(raw_scale[0] != raw_scale[0]){
        // NaN. skip frame
        
        // Clear graphs if NaN and on output mode (usually means file not playing)
        return false;
        // Sometimes buffer will be NaN during input lag
        // So if we reset the graph each time (on input mode) it would be jumpy
        // Haven't observed the same issue on output
    }
    
    // At the moment, smoothing consists of:
    //   - rolling average to make it less 'jumpy'
    for(int i=0; i<oct_size; i++){
        //raw_octave[i] = ofClamp(raw_octave[i], 0, 1); // clamp new data
        smooth_octave[i] = approxRollingAverage(smooth_octave[i], raw_octave[i]);
        if(smooth_octave[i] < 0.3) smooth_octave[i] *= smooth_octave[i];
    }
    
    for(int i=0; i<scale_size; i++){
        //raw_scale[i] = ofClamp(raw_scale[i], 0, 1); // clamp new data
        if(addOvertone){
            float overtone = 0;
            int count = 0;
            for(int j=i+12; j<scale_size; j+=12){
                overtone += raw_scale[j];
                count += 1;
            }
            if(count > 0) {
                overtone /= count;
                smooth_scale[i] = approxRollingAverage(smooth_scale[i], (raw_scale[i]+overtone)/2);
            }
            else{
                smooth_scale[i] = approxRollingAverage(smooth_scale[i], raw_scale[i]);
            }
        }
        else{
            smooth_scale[i] = approxRollingAverage(smooth_scale[i], raw_scale[i]);
        }
    }
    
    return true;
}


//--------------------------------------------------------------
bool Analysis::isFrameReady(){
    return frameReady;
}


//--------------------------------------------------------------
float* Analysis::getRawOctave(){
    return raw_octave;
}


//--------------------------------------------------------------
float* Analysis::getRawScale(){
    return raw_scale;
}

//--------------------------------------------------------------
float* Analysis::getOctave(){
    return smooth_octave;
}

//--------------------------------------------------------------
float* Analysis::getScale(){
    return smooth_scale;
}

//--------------------------------------------------------------
float* Analysis::getFft(){
    return raw_fft;
}

//--------------------------------------------------------------
int Analysis::getFftSize(){
    return fft_size;
}


//--------------------------------------------------------------
int Analysis::getOctaveSize(){
    return oct_size;
}

//--------------------------------------------------------------
int Analysis::getScaleSize(){
    return scale_size;
}

//--------------------------------------------------------------
void Analysis::setAddOvertone(bool b){
    addOvertone = b;
}

