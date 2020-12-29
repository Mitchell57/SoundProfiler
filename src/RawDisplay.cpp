//
//  RawDisplay.cpp
//  SoundProfiler
//
//  Created by Mitch on 12/29/20.
//

#include <stdio.h>
#include "RawDisplay.h"

RawDisplay::RawDisplay(){
    name = "FFT";
}

void RawDisplay::setup(){
    
    
    parameters.setName("Raw Controls");
    parameters.add(linlog.set("Linear / Logarithmic", true));
    
    
    dataRequest.push_back(utils::RAW_FULL);
}


void RawDisplay::draw(){
    ofPushMatrix();
    ofTranslate(xOffset,yOffset);
    drawFftPlot(halfW, halfH);
    ofPopMatrix();
    

}


void RawDisplay::update(std::vector<utils::soundData> newData){
    
    for(utils::soundData container : newData){
        switch (container.label) {
            case utils::RAW_FULL:
                raw_fft = container.data;
                break;

            default:
                break;
        }
    }
    
}
void RawDisplay::setDimensions(int w, int h){
    width = w;
    height = h;
    
    halfW = ((float)w*0.9);
    halfH =  ((float)h*0.425);

    xOffset = h*0.05;
    yOffset = w*0.05;
}


void RawDisplay::drawFftPlot(int w, int h){
    
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
    std::string label = "FFT Plot";
    if(yOffset > 20) ofDrawBitmapString(label, 0, -8);
    ofPopStyle();
    
    if(raw_fft.size() <= 1) return;
    float x = 0;
    float y = 0;
    
    float x_inc = (((float)w) * 0.99) / (float)raw_fft.size();
    margin = (((float)w) - x_inc*raw_fft.size()) / 2;
    maxHeight = ((float)h)*0.95;
    y_offset = (float)(h + maxHeight)/2;
    
    ofSetColor(255);
    ofPath path;
    
    if(linlog){
        ofPushMatrix();
        ofTranslate(0, y_offset); //Move to bottom-left corner for start
        
        path.moveTo(0, -raw_fft[0]*maxHeight);
        
        //loop through values
        for(int i=1; i<raw_fft.size(); i++){
            x += x_inc;
            y = -raw_fft[i]*maxHeight;
            path.lineTo(x, y);
        }
        
        path.setFilled(false);
        path.setStrokeColor(ofColor::white);
        path.setStrokeWidth(1);
        path.setCircleResolution(20);
        path.draw();
        
        ofPopMatrix();
    }
    else{
        ofPushMatrix();
        
        
        path.moveTo(log2f(margin), -raw_fft[0]*maxHeight);
        x = margin;
        float max_y = 0;
        //loop through values
        for(int i=1; i<raw_fft.size(); i++){
            x += x_inc;
            y = -raw_fft[i]*maxHeight;
            if(y > max_y) max_y = y;
            path.lineTo(log2f(x), y);
        }
        
        std::vector<ofPolyline> outline = path.getOutline();
        ofRectangle bb = outline[0].getBoundingBox();
        float pw = bb.getWidth();
        float ph = bb.getHeight();
        path.scale(((float)w)/pw, maxHeight / ph);
        outline = path.getOutline();
        bb = outline[0].getBoundingBox();
        pw = bb.getWidth();
        ph = bb.getHeight();
        float px = bb.getX();
        float py = bb.getY();
        
        ofTranslate(-(px), y_offset);
        path.setFilled(false);
        path.setStrokeColor(ofColor::white);
        path.setStrokeWidth(1);
        
        path.draw();
        
        ofPopMatrix();
    }
}


