//
//  RawDisplay.cpp
//  SoundProfiler
//
//  Created by Mitch on 12/29/20.
//

#include <stdio.h>
#include "RawDisplay.h"

RawDisplay::RawDisplay(){
    name = "Frequency";
}

void RawDisplay::setup(){
    dataRequest.push_back(utils::RAW_FULL);

    labelGap = 35;
    numLabels = 4;
}

void RawDisplay::buildGui(ofxGuiGroup* parent){
    group = parent->addGroup("Raw Controls");
    group->setShowHeader(false);
    
    linLogParameters.setName("X-Axis Scale");
    linLogParameters.add(lin.set("Linear", false));
    linLogParameters.add(log.set("Logarithmic", false));
    
    
    linLogToggles = group->addGroup(linLogParameters);
    linLogToggles->setExclusiveToggles(true);
    linLogToggles->setConfig(ofJson({{"type", "radio"}}));
    //linLogToggles->getActiveToggleIndex().addListener(this, &RawDisplay::setRawLinLog);
    linLogToggles->setActiveToggle(0);
    
    freqsliderStart.set("range",0,0,22000); //use the first parameter to set the initial lower value and the min and max value
    freqsliderEnd.set(15000); // use the second parameter to set the initial upper value
    group->add<ofxGuiIntRangeSlider>(freqsliderStart, freqsliderEnd, ofJson());
    
    freqCenter.set("Window Center", 11025, 0, 22050);
    freqWidth.set("Window Width", 100, 0, 100);
    group->add<ofxGuiFloatSlider>(freqCenter, ofJson({{"precision", 0}}));
    group->add<ofxGuiFloatSlider>(freqWidth, ofJson({{"precision", 0}}));

    
    group->add(reset.set("Reset Settings"), ofJson({{"type", "fullsize"}, {"text-align", "center"}}));
    reset.addListener(this, &RawDisplay::resetParameters);
    
}

void RawDisplay::resetParameters(){
    freqCenter.set(11025);
    freqWidth.set(100);
}


void RawDisplay::draw(){
    ofPushStyle();
    ofPushMatrix();
    ofTranslate(xOffset,yOffset);
    drawFftPlot(halfW, halfH);
    ofPopMatrix();
    ofPopStyle();

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
    
    float max = 0;
    for(float val : raw_fft){
        if(val > max) max = val;
    }
    for(float val : raw_fft){
        val /= max;
    }
    
    
}
void RawDisplay::setDimensions(int w, int h){
    width = w;
    height = h;
    
    halfW = ((float)width*0.9);
    halfH =  ((float)height*0.425);

    xOffset = width*0.05;
    yOffset = height*0.05;
}


void RawDisplay::drawFftPlot(int w, int h){
    if(raw_fft.size() <= 1) return;
    
    
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
    
    float midBin = (freqCenter*raw_fft.size())/22050.;
    float binRadius = ((freqWidth/100.)*raw_fft.size())/2;
    

    
    int startBin = midBin-binRadius;
    int endBin = midBin+binRadius;
    
    
    
    
    
    float x_inc = (w * 0.95) / (2*binRadius);
    
    float x = 0;
    float y = 0;
    
    margin = (w * 0.05) / 2;
    maxHeight = h*0.95;
    y_offset = (h + maxHeight)/2;
    
    ofSetColor(255);
    ofPath path;
    
    float xLabel = 0;
    float label_inc = labelGap;
    int labelCount = 0;
    float numLines = width / labelGap;
    int labelSignal = max((int)(numLines/numLabels), 2);
    
    if(lin){
        ofPushMatrix();
        ofTranslate(margin, y_offset); //Move to bottom-left corner for start
        
        labelGap = 50;
        
        //loop through values
        for(int i=startBin; i<endBin; i++){
            if(i>=0 && i < raw_fft.size()){
                y = -raw_fft[i]*maxHeight;
                if(label_inc >= labelGap){
                    label_inc = 0;
                    float freq = ((22050*i)/(raw_fft.size()));
                    freq = round(freq/10.) * 10;
                    std::string label;
                    if(freq >= 1000){
                        freq /= 1000.;
                        std::stringstream stream;
                        stream << std::fixed << std::setprecision(1) << freq;
                        label = stream.str()+"k";
                    }
                    else {
                        std::stringstream stream;
                        stream << std::fixed << std::setprecision(0) << freq;
                        label = stream.str();
                    }
                    
                    if(labelCount%labelSignal == 0) {
                        ofSetColor(100);
                        ofDrawBitmapString(label, x+5, -maxHeight+10);
                    }
                    ofSetColor(70);
                    ofDrawLine(x, 0, x, -maxHeight);
                    
                }
            }
            else y = 0;
            
            path.lineTo(x, y);
            
            
        
            labelCount += 1;
            x += x_inc;
            label_inc += x_inc;
            
        }
        
        path.setFilled(false);
        path.setStrokeColor(ofColor::white);
        path.setStrokeWidth(1);
        //utils::scalePath(&path, w*0.95, maxHeight);
        path.draw();
        
        ofPopMatrix();
    }
    else{
        ofPushMatrix();
        float maxX = (endBin)*x_inc;
        float x_scale = (maxX) / (logf(maxX) - logf(margin));
        
        labelGap = 20;
        x = margin;
        //loop through values
        for(int i=startBin; i<endBin; i++){
            
            y = -raw_fft[i]*maxHeight;
            
            if(i==0) path.moveTo(logf(margin), y);
            path.lineTo(logf(x), y);
            
            if(label_inc >= labelGap){
                label_inc = 0;
                int freq = ((22050*i)/(raw_fft.size()));
                freq -= (freq%100);
                std::string label;
                if(freq >= 1000){
                    freq /= 1000.;
                    std::stringstream stream;
                    stream << std::fixed << std::setprecision(1) << freq;
                    std::string s = stream.str();
                    label = s+"k";
                }
                else {
                    label = std::to_string(freq);
                }
                
                float lineX = margin+x_scale*(logf(x) - logf(margin));
                
                float nextX = margin+x_scale*(logf(x+(x_inc*labelGap)) - logf(margin));
                
                
                if((nextX-lineX) > labelGap) {
                    ofSetColor(100);
                    ofDrawBitmapString(label, lineX+3, 15);
                    
                }
                ofSetColor(70);
                ofDrawLine(lineX, 0, lineX, h);
            }
            
            x += x_inc;
            label_inc += x_inc;
            labelCount += 1;
        }
        
        utils::scalePath(&path, w*0.95, maxHeight);
        ofRectangle bb = path.getOutline().front().getBoundingBox();
        float px = bb.getX();
        float py = bb.getY();
        
        ofTranslate(margin-px, y_offset);
        path.setFilled(false);
        path.setStrokeColor(ofColor::white);
        path.setStrokeWidth(1);
        
        path.draw();
        
        ofPopMatrix();
    }
}


