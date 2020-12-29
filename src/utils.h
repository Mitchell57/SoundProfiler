//
//  utils.h
//  SoundProfiler
//
//  Created by Mitch on 12/29/20.
//

#ifndef utils_h
#define utils_h
#include <vector>

namespace utils {

    // Approximate Rolling Average from:
    // https://bit.ly/3aIsQRD
    float approxRollingAverage (float avg, float new_sample, float n) {

        avg -= avg / 3;
        avg += new_sample / 3;

        return avg;
    }

    enum Mode{ LINEAR, POLAR, RAW, OSC };

    enum soundType{ RAW_FULL, RAW_OCTAVE, SMOOTH_OCTAVE, RAW_SCALE, SMOOTH_SCALE, SMOOTH_SCALE_OT };

    struct soundData {
        soundType label;
        std::vector<float> data;
    };

}



#endif /* utils_h */
