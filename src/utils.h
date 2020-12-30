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
    static float approxRollingAverage(float avg, float new_sample, float n) {

        avg -= avg / n;
        avg += new_sample / n;

        return avg;
    }

    enum Mode{ LINEAR, POLAR, RAW, OSC };

    enum soundType{ RAW_FULL, RAW_OCTAVE, SMOOTH_OCTAVE, RAW_SCALE, SMOOTH_SCALE, SMOOTH_SCALE_OT };

    struct soundData {
        soundType label;
        std::vector<float> data;
    };

    static void scalePath(ofPath* path, float width, float height){
        std::vector<ofPolyline> outline = path->getOutline();
        ofRectangle bb = outline[0].getBoundingBox();
        
        float p_width = bb.getWidth();
        float p_height = bb.getHeight();
        
        path->scale(width/p_width, height/p_height);
    }

}



#endif /* utils_h */
