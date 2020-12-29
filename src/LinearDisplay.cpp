//
//  LinearDisplay.cpp
//  SoundProfiler
//
//  Created by Mitch on 12/29/20.
//

#include <stdio.h>
#include "LinearDisplay.h"
/*
 From Display.h
 
 ofParameterGroup parameters;
 ofxGuiGroup *group;
 std::vector<utils::soundType> dataRequest;
 std::string name;
 int width, height;
*/


LinearDisplay::LinearDisplay(){
    name = "Linear";
}

void LinearDisplay::setup(){
    
    
    parameters.setName("Linear Controls");
    parameters.add(overtoneToggle.set("Factor Overtones", false));
    parameters.add(colorToggle.set("Color", true));
    
    dataRequest = {utils::SMOOTH_OCTAVE, utils::SMOOTH_SCALE };
}


void LinearDisplay::draw(){
    ofPushMatrix();
    ofTranslate(xOffset,yOffset);
    drawLinOctave(halfW, halfH);
    ofPopMatrix();
    
    ofPushMatrix();
    ofTranslate(xOffset, halfH+(2*yOffset));
    drawLinScale(halfW, halfH);
    ofPopMatrix();
}


void LinearDisplay::update(std::vector<utils::soundData> newData){
    if(overtoneToggle) dataRequest[1] = utils::SMOOTH_SCALE_OT;
    else dataRequest[1] = utils::SMOOTH_SCALE;
    
    for(utils::soundData container : newData){
        switch (container.label) {
            case utils::SMOOTH_SCALE:
            case utils::SMOOTH_SCALE_OT:
                scale = container.data;
                break;
                
            case utils::SMOOTH_OCTAVE:
                octave = container.data;
                break;
            default:
                break;
        }
    }
    
}
void LinearDisplay::setDimensions(int w, int h){
    width = w;
    height = h;
    
    halfW = ((float)w*0.9);
    halfH =  ((float)h*0.425);

    xOffset = h*0.05;
    yOffset = w*0.05;
}

//--------------------------------------------------------------------------------------
// linear summed-octave
//--------------------------------------------------------------------------------------
void LinearDisplay::drawLinOctave(int w, int h){
    
    // Draw border
    ofPushStyle();
    ofSetColor(ofColor::white);
    ofNoFill();
    ofRectangle outer_rect;
    outer_rect.x = 0;
    outer_rect.y = 0;
    outer_rect.width = w;
    outer_rect.height = h;
    ofDrawRectangle(outer_rect);
    std::string label = "Summed Octave";
    if(yOffset > 20) ofDrawBitmapString(label, 0, -8);
    ofPopStyle();
    
    if(octave.size() <= 1) return;
    // Initialize graph values
    //    Data = Summed Octave
    //    Bars take up 80% of total width
    //    Margins take up rest of total width
    //    Max bar height is 95% of height
    //    Labels are displayed if bar width > 12 pixels
    
    barWidth = (((float)w) * 0.8) / (float)octave.size();
    margin = (((float)w) - barWidth*octave.size()) / ((float)octave.size()+1);
    maxHeight = (-20)+((float)h)*0.95;
    y_offset = (float)(h + maxHeight)/2;
    bool labelsOn = (barWidth > 12);
    
    
    int x = 0;
    int noteNum = 0;
    int octaveNum = 0;
    int labelXOffset = max((barWidth-15)/2, 0); // labels are ~15px, so offset centers them
    int yPosLabel;
    
    ofPushMatrix();
    ofTranslate(margin, y_offset); //Move to bottom-left corner for start
    
    //loop through raw values
    for(int i=0; i<octave.size(); i++){
        
        ofPushStyle();
        
        ofRectangle rect;
        rect.x = x;
        rect.y = 0;
        rect.width = barWidth;

        // If rectangle height is below min. threshold, draw min rectangle
        // Note label follows rectangle if possible, otherwise sits on top of min rect.
        // y-axis is 'flipped' i.e. negative is upwards
        if(octave[i] < 0.05 || octave[i] > 1.0 || octave[i] != octave[i]) {
            rect.height =  -3;
            yPosLabel = -6;
        }
        else{
            rect.height = -octave[i]*maxHeight;
            yPosLabel = std::max((int)rect.height-6, (int)-maxHeight-10);
        }
        
        
        float hue = (i%12)*(255.0/12);
        float sat = 100+octave[i%12]*155;
        if(!colorToggle) sat = 0;
        float brightness = 55+octave[i]*200;
        
        ofColor color = ofColor::fromHsb(hue, sat, brightness);
        ofSetColor(color);
        
        ofDrawRectangle(rect);
        ofPopStyle();
        
        // Draw note label
        if(labelsOn){
            ofSetColor(ofColor::white);
            std::string label = noteNames[noteNum];
            ofDrawBitmapString(label, x+labelXOffset, yPosLabel);
        }
        
        // increment x position, note, and octave (if necessary)
        x += barWidth+margin;
        noteNum++;
        if(noteNum > 11){
            noteNum = 0;
        }
    }
    
    ofPopMatrix();
}


//--------------------------------------------------------------------------------------
// linear full-scale
//--------------------------------------------------------------------------------------
void LinearDisplay::drawLinScale(int w, int h){
    
    // Draw border
    ofPushStyle();
    ofSetColor(ofColor::white);
    ofNoFill();
    ofRectangle outer_rect;
    outer_rect.x = 0;
    outer_rect.y = 0;
    outer_rect.width = w;
    outer_rect.height = h;
    ofDrawRectangle(outer_rect);
    std::string label = "Full Scale";
    if(yOffset > 20) ofDrawBitmapString(label, 0, -8);
    ofPopStyle();
    
    if(scale.size() <= 1) return;
    // Initialize graph values
    //    Data = Summed Octave
    //    Bars take up 80% of total width
    //    Margins take up rest of total width
    //    Max bar height is 95% of height
    //    No labels
    barWidth = (((float)w) * 0.85) / (float)scale.size();
    margin = (((float)w) - barWidth*scale.size()) / ((float)scale.size()-1);
    float edgeMargin = ((float)w - (barWidth*scale.size() + margin*(scale.size()-1))) / 2;
    maxHeight = ((float)h)*0.95;
    y_offset = (float)(h + maxHeight)/2;
    
    int x = 0;
    int noteNum = 0;
    int octaveNum = 2;
    
    ofPushMatrix();
    ofTranslate(edgeMargin, y_offset); //Move to bottom-left corner for start
    
    //loop through raw values
    for(int i=0; i<scale.size(); i++){
        
        ofPushStyle();
        
        
        ofRectangle rect;
        rect.x = x;
        rect.y = 0;
        rect.width = barWidth;

        // If rectangle height is below min. threshold, draw min rectangle
        // y-axis is 'flipped' i.e. negative is upwards
        if(scale[i] < 0.05 || scale[i] > 1.0 || scale[i] != scale[i]) {
            rect.height =  -3;
        }
        else{
            rect.height = -scale[i]*maxHeight;
        }
        
        float hue = (i%12)*(255.0/12);
        
        float sat = 100+scale[i]*155;
        if(!colorToggle) sat = 0;
        float brightness = 55+scale[i]*200;
        

        
        ofColor color = ofColor::fromHsb(hue, sat, brightness);
        ofSetColor(color);
        
        ofDrawRectangle(rect);
        ofPopStyle();
        
        // increment x position, note, and octave (if necessary)
        x += barWidth+margin;
    }
    
    ofPopMatrix();
}

