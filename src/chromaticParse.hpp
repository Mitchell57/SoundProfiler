//
//  chromaticParse.hpp
//  soundProfiler
//
//  Created by Mitch on 10/26/20.
//

#ifndef chromaticParse_hpp
#define chromaticParse_hpp

#include <stdio.h>
#include <vector>

struct range {
    int low;
    int high;
};

struct key {
    int keynum;
    float level;
};


enum viewMode {
    VIEW_BARK,
    VIEW_HARMONICS,
    VIEW_CHROMATIC
};

class ChromaticParse {
public:
    
    ChromaticParse();
    ~ChromaticParse();
    
    void init(int nbins);

    std::vector<int> getKeys();
    std::vector<float> getDeltas();
    std::vector<float> getRaw();
    std::vector<float> getOctave();
    void updateData(float* bins);
    int getRawSize();
    int getCSize();
    std::vector<int> keyList;
    
    
private:
    int hlen, numBins;
    // Frequencies of 12-note chromatic scale (A4 - Ab4)
    std::vector<float> chromaticScale = {440, 466.16, 493.88, 523.25, 554.37, 587.33, 622.25, 659.26, 698.46, 739.99, 783.99, 830.61};
    
    
    std::vector<float> rawData;

    int c_size;
    float delta_max, delta_sum, threshold;
    
    std::vector<float> singleOctave;
    std::vector<float> raws;
    std::vector<float> oldRaw;
    std::vector<float> deltas;
    std::vector<key> keysPressed;
    std::vector<int> chromaticBins;

    
    void parseData();
};

#endif /* chromaticParse_hpp */
