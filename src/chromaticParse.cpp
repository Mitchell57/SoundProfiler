//
//  chromaticParse.cpp
//  soundProfiler
//
//  Created by Mitch on 10/26/20.
//
#include <math.h>
#include "chromaticParse.hpp"


int freq2bin(float freq, int numBins){
    float mult = (float)numBins / 22000;
    return floor(freq*mult);
    
}

// Constructor / Destructor
ChromaticParse::ChromaticParse(){}
ChromaticParse::~ChromaticParse(){}




void ChromaticParse::init(int nbins, std::vector<int> binlist){

    numBins = nbins;
    chromaticBins = binlist;
    
    // Populate binList with 7 octaves of chromatic scale (centered at octave starting with A=440Hz)
    // Each octave is 2x the frequency of previous octave,
    // So by finding bin# of scale * successive powers of 2 we can obtain the bins
    // needed to find amplitude along a contiguous portion of chromatic scale
    
    
    singleOctave.resize(12);
    c_size = chromaticBins.size();
    raws.resize(c_size);
    delta_max = 0.0000001;
}


// Receives new FFT data and updates raw values accordingly
void ChromaticParse::updateData(float* bins){
    
    float frame_max = 0;
    
    delta_sum = 0;
    float raw, harmonic;
    for(int i=0; i<c_size; i++){
        raw = bins[chromaticBins[i]];
        harmonic = raw;
        for(int j=i+12; j<c_size; j+=12){
            harmonic += bins[chromaticBins[j]];
        }
        if(raw > frame_max) frame_max = raw;
        if(harmonic > frame_max) frame_max=harmonic;
        raws[i] = raw*5;
        //deltas[i] = harmonic;
    }
    
    
    for(int i=0; i<c_size; i++){
        //deltas[i] /= frame_max;
    }

    float single_max = 0;
    for(int i=0; i<singleOctave.size(); i++){
        float val = 0;
        for(int j=i; j<c_size; j+=12){
            val += bins[chromaticBins[j]];
        }
        singleOctave[i] = val;
        if(val > single_max) single_max = val;
    }
    for(int i=0; i<singleOctave.size(); i++){
        singleOctave[i] /= single_max;
    }
    parseData();
}

std::vector<float> ChromaticParse::getOctave(){
    return singleOctave;
}

std::vector<float> ChromaticParse::getRaw(){
    return raws;
}


int ChromaticParse::getCSize(){
    return c_size;
}

void ChromaticParse::parseData(){
    //stub
}
