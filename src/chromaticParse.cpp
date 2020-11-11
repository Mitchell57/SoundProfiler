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




void ChromaticParse::init(int nbins){

    numBins = nbins;
    
    
    // Populate binList with 7 octaves of chromatic scale (centered at octave starting with A=440Hz)
    // Each octave is 2x the frequency of previous octave,
    // So by finding bin# of scale * successive powers of 2 we can obtain the bins
    // needed to find amplitude along a contiguous portion of chromatic scale
    for(int i=-2; i<=3; i++){
        for(int j=0; j<chromaticScale.size(); j++){
            float freq = chromaticScale[j]*pow(2, i);
            chromaticBins.push_back(freq2bin(freq, nbins));
            oldRaw.push_back(0.0);
            keyList.push_back(0);
        }
    }
    
    singleOctave.resize(12);
    c_size = chromaticBins.size();
    oldRaw.resize(c_size);
    raws.resize(c_size);
    deltas.resize(c_size);
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
        deltas[i] = harmonic;
    }
    
    
    for(int i=0; i<c_size; i++){
        deltas[i] /= frame_max;
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

std::vector<int> ChromaticParse::getKeys(){
    return keyList;
}

std::vector<float> ChromaticParse::getOctave(){
    return singleOctave;
}

std::vector<float> ChromaticParse::getRaw(){
    return raws;
}

std::vector<float> ChromaticParse::getDeltas(){
    return deltas;
}

int ChromaticParse::getCSize(){
    return c_size;
}

void ChromaticParse::parseData(){
    //stub
}
