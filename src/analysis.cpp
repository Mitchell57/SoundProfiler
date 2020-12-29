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

    avg -= avg / 3;
    avg += new_sample / 3;

    return avg;
}


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
    raw_fft = new float[fft_size];
    
    raw_octave = new float[oct_size];
    raw_scale = new float[scale_size];
    
    smooth_octave = new float[oct_size];
    smooth_scale = new float[scale_size];
    
    for(int i=0; i<oct_size; i++){
        raw_octave[i] = 0.001;
        smooth_octave[i] = 0.001;
    }
    
    for(int i=0; i<scale_size; i++){
        raw_scale[i] = 0.001;
        smooth_scale[i] = 0.001;
    }
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
    
    // Retrieve analyzed frame
    raw_fft = fft->getAmplitude();
    
    float fft_max = 0;
    for(int i=0; i<fft_size; i++){
        if(raw_fft[i] > fft_max) fft_max = raw_fft[i];
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
        
        val = raw_fft[fullBinList[i]];
        
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
    
    float newVal, overtone;
    for(int i=0; i<scale_size; i++){
        newVal = raw_scale[i];
        if(addOvertone){
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
        }
        
        smooth_scale[i] = approxRollingAverage(smooth_scale[i], newVal);
    }
    
    return true;
}


//---------------------------------------------------------------------------
// getters & setters
//---------------------------------------------------------------------------

//--------------------------------------------------------------
bool Analysis::isFrameReady(){ return frameReady; }

//--------------------------------------------------------------
float* Analysis::getRawOctave(){ return raw_octave; }

//--------------------------------------------------------------
float* Analysis::getRawScale(){ return raw_scale; }

//--------------------------------------------------------------
float* Analysis::getOctave(){ return smooth_octave; }

//--------------------------------------------------------------
float* Analysis::getScale(){ return smooth_scale; }

//--------------------------------------------------------------
float* Analysis::getFft(){ return raw_fft; }

//--------------------------------------------------------------
int Analysis::getFftSize(){ return fft_size; }

//--------------------------------------------------------------
int Analysis::getOctaveSize(){ return oct_size; }

//--------------------------------------------------------------
int Analysis::getScaleSize(){ return scale_size; }

//--------------------------------------------------------------
void Analysis::setAddOvertone(bool b){ addOvertone = b; }

