//
//  DisplayController.cpp
//  SoundProfiler
//
//  Created by Mitch on 12/29/20.
//

#include <stdio.h>
#include "DisplayController.h"



DisplayController::DisplayController(){}

void DisplayController::setup(Analysis* a, int w, int h, ofxGuiGroup* all){
    analysis = a;
    
    current_mode = 0;
    
    LinearDisplay* ld1 = new LinearDisplay();
    ld1->setup();
    modes.push_back(ld1);
    
    LinearDisplay* ld2 = new LinearDisplay();
    ld2->setup();
    modes.push_back(ld2);
    
    modeSelector.setName("Display Mode");
    modeSelector.add(disp0.set(ld1->name,true));
    modeSelector.add(disp1.set(ld2->name,false));

    
    modeSelectorGroup = all->addGroup(modeSelector);
    modeSelectorGroup->setExclusiveToggles(true);
    modeSelectorGroup->loadTheme("default-theme.json");
    modeSelectorGroup->setConfig(ofJson({{"type", "radio"}}));
    
    modeControlGroup = all->addGroup("Mode Controls");

    for(int i=0; i<modes.size(); i++){
        
        modes[i]->setDimensions(10, 10); // STUB

        modeControls.push_back(modeControlGroup->addGroup(modes[i]->parameters));
        modeControls[i]->setShowHeader(false);
    }
        
    
    
    
    // set up modes
    // build selector list
    //
    modeSelectorGroup->getActiveToggleIndex().addListener(this, &DisplayController::setDisplayMode);
    modeSelectorGroup->setActiveToggle(0);

    
    
    ready = true;
    
}

void DisplayController::setDisplayMode(int& index){
    modeControlGroup->minimizeAll();
    switch(index){
            default: case 0:
            current_mode = 0;
            modeControls[0]->maximize();
                break;
            case 1:
            current_mode = 1;
            modeControls[1]->maximize();
                break;
            case 2:
                
                break;
        }
}


void DisplayController::draw(){
    if(modes[current_mode] != NULL){
        modes[current_mode]->draw();
    }
}

void DisplayController::minimize(){
    modeSelectorGroup->minimize();
    modeControlGroup->minimize();
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
