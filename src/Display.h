//
//  Display.h
//  SoundProfiler
//
//  Created by Mitch on 12/29/20.
//
#include "ofxGuiExtended.h"
#include "utils.h"

#ifndef Display_h
#define Display_h

class Display{
public:
    virtual void draw() = 0;
    virtual void update(std::vector<utils::soundData> newData) = 0;
    virtual void setup() = 0;
    virtual void setDimensions(int w, int h) = 0;
    
    std::string name;
    ofParameterGroup parameters;
    
    std::vector<utils::soundType> dataRequest;
    
protected:
    int width, height;
};

#endif /* Display_h */
