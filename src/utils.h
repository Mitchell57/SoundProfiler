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

    enum Mode{ LINEAR, POLAR, RAW, OSC };

    enum soundType{ RAW_FULL, RAW_OCTAVE, SMOOTH_OCTAVE, RAW_SCALE, SMOOTH_SCALE, SMOOTH_SCALE_OT };

    struct soundData {
        soundType label;
        std::vector<float> data;
    };


    // Approximate Rolling Average from:
    // https://bit.ly/3aIsQRD
    static float approxRollingAverage(float avg, float new_sample, float n) {

        avg -= avg / n;
        avg += new_sample / n;

        return avg;
    }


    static void scalePath(ofPath* path, float width, float height){
        std::vector<ofPolyline> outline = path->getOutline();
        ofRectangle bb = outline[0].getBoundingBox();
        
        float p_width = bb.getWidth();
        float p_height = bb.getHeight();
        
        path->scale(width/p_width, height/p_height);
    }


    static string formatFreq(float freq){
        std::string label;

        if(freq > 1000){
            freq /= 1000.;
            std::stringstream stream;
            stream << std::fixed << std::setprecision(2) << freq;
            label = stream.str()+"k";
        }
        else{
            freq = round(freq / 10.)*10;
            std::stringstream stream;
            stream << std::fixed << std::setprecision(0) << freq;
            label = stream.str();
        }
    
        return label;
    }


    // Returns width in pixels of a given string when drawn as a bitmap
    // adapted from https://forum.openframeworks.cc/t/how-to-get-size-of-ofdrawbitmapstring/22578/7
    static int getBitmapStringWidth(string text){
    vector<string> lines = ofSplitString(text, "\n");
        int maxLineLength = 0;
        for(int i = 0; i < (int)lines.size(); i++) {
            // tabs are not rendered
            const string & line(lines[i]);
            int currentLineLength = 0;
            for(int j = 0; j < (int)line.size(); j++) {
                if (line[j] == '\t') {
                    currentLineLength += 8 - (currentLineLength % 8);
                } else {
                    currentLineLength++;
                }
            }
            maxLineLength = MAX(maxLineLength, currentLineLength);
        }
        
        int fontSize = 8;

        return maxLineLength * fontSize;
    }

}





#endif /* utils_h */
