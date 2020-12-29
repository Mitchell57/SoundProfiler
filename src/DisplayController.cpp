//
//  DisplayController.cpp
//  SoundProfiler
//
//  Created by Mitch on 12/29/20.
//

#include <stdio.h>
#include "DisplayController.h"



DisplayController::DisplayController(){}

void DisplayController::setup(Analysis* a, int w, int h){
    analysis = a;
    
    current_mode = 0;
    
    LinearDisplay* ld = new LinearDisplay();
    modes.push_back(ld);
    
    parameters.setName("Mode Controls");
    for(int i=0; i<modes.size(); i++){
        modes[i]->setup();
        modes[i]->setDimensions(10, 10); // STUB
        parameters.add(modes[i]->parameters);
    }
    // set up modes
    // build selector list
    //
    ready = true;
    
}

void DisplayController::draw(){
    if(modes[current_mode] != NULL){
        modes[current_mode]->draw();
    }
}

void DisplayController::update(){
    if(modes[current_mode] != NULL){
        std::vector<utils::soundData> ret;
        for(utils::soundType req : modes[current_mode]->dataRequest){
            utils::soundData container;
            container.label = req;
            float* inData = analysis->getData(req);
            int inSize = analysis->getSize(req);
            std::vector<float> inVec = {inData, inData+inSize};
            container.data = inVec;
            
            ret.push_back(container);
        }
        
        modes[current_mode]->update(ret);
    }
}

void DisplayController::updateLayout(int w, int h){
    width = w;
    height = h;
    
    for(Display* mode : modes){
        mode->setDimensions(w, h);
    }
}
void DisplayController::setMode(int index){
    // STUB
}
int DisplayController::getMode(){
    return 0; // STUB
}


//    // general control

//
//    // mode selection
    
//
//    ofParameterGroup parameters;
//
//    ofxGuiGroup *modeSelectorGroup; // add mode buttons
//
//    ofxGuiGroup *modeControlGroup; // add all mode-specific parameters, only show current mode
//
//
//protected:
//    std::vector<Display> modes;
//
//    int width, height;
//    int current_mode;
//    Analysis* analysis;
//}
