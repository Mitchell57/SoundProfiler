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
    startBin = 0;
    endBin = 1025;
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
    

    windowGroup = group->addGroup("FFT Window");
    freqCenter.set("Window Center", 11025, 1, 22050);
    freqWidth.set("Window Width", 22050, 1, 22050);
    smooth.set("Smoothing", 1., 1., 5.);
    windowGroup->add<ofxGuiFloatSlider>(freqCenter, ofJson({{"precision", 0}}));
    windowGroup->add<ofxGuiFloatSlider>(freqWidth, ofJson({{"precision", 0}}));
    windowGroup->add<ofxGuiFloatSlider>(smooth, ofJson({{"precision", 1}}));
    windowGroup->add(rescale.set("Rescale Window", false));

    
    windowGroup->add(reset.set("Reset Settings"), ofJson({{"type", "fullsize"}, {"text-align", "center"}}));
    reset.addListener(this, &RawDisplay::resetParameters);
    freqWidth.addListener(this, &RawDisplay::fftWindowChanged);
    freqCenter.addListener(this, &RawDisplay::fftWindowChanged);
    
    
}

void RawDisplay::fftWindowChanged(float& val){
    float midBin = (freqCenter*raw_fft.size())/22050.;
    float binRadius = ((freqWidth/22050.)*raw_fft.size())/2;
        
    startBin = midBin-binRadius;
    endBin = midBin+binRadius;
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
                raw_fft.resize(container.data.size());
                for(int i=0; i<raw_fft.size(); i++){
                    raw_fft[i] = utils::approxRollingAverage(raw_fft[i], container.data[i], smooth);
                }
                break;

            default:
                break;
        }
    }
    
    std::lock_guard<std::mutex> guard(mtx);
    size = endBin - startBin;
    fft_display.clear();
    fft_display_freqs.clear();
    // Update display data
    for(int i=startBin; i<endBin; i++){
        if(i < 0 || i >= raw_fft.size()){
            fft_display.push_back(0);
            fft_display_freqs.push_back(-1);
        }
        else{
            fft_display.push_back(raw_fft[i]);
            fft_display_freqs.push_back(i);
        }
    }
    
    if(rescale){
        float max = 0;
        for(float val : fft_display){
            if(val > max) max = val;
        }
        if(max != 0){
            for(int i=0; i<fft_display.size(); i++){
                fft_display[i] /= max;
            }
        }
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

void RawDisplay::drawLinearFftWindow(float w, float h){
    ofPushMatrix();
    ofTranslate(0, h);
    
    ofPolyline plot;
    plot.begin();
    //plot.addVertex(w, 0);
    //plot.addVertex(0, 0);
    
    float x,y;
    x = 0;
    
    bool lineB = true;
    bool labelB = true;
    
    float numLines = 10;
    float numLabels = 5;
    
    std::lock_guard<std::mutex> guard(mtx);
    for(int i=0; i<size; i++){
        
        x = (((float)i) / size) * w;
        y = -(h * fft_display[i]);
        
        plot.addVertex(x, y);
        if((int)(size/numLines) == 0){
            lineB = true;
            labelB = true;
        }
        else{
            lineB = ((i % (int)(size / numLines)) == 0);
            labelB = ((i % (int)(size / numLabels)) == 0);
        }
        
        
        if(lineB && !labelB){
            ofSetColor(30);
            ofDrawLine(x, 0, x, -h);
            
        }
        if(labelB){
            std::string label = utils::labelFromBin(fft_display_freqs[i], raw_fft.size());
            ofSetColor(120);
            ofDrawBitmapString(label, x-2, -h);
            ofSetColor(50);
            ofDrawLine(x, 0, x, -h*0.95);
        }
    }
    
    //plot.close();
    ofSetColor(255);
    ofFill();
    plot.getSmoothed(2).draw();
    
    ofPopMatrix();
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
    
    ofPushMatrix();
    ofTranslate(w*0.025, h*0.05);
    drawLinearFftWindow(w*0.95, h*0.9);
    ofPopMatrix();
    
//    else{
//        ofPushMatrix();
//        float maxX = (endBin)*x_inc;
//        float x_scale = (maxX) / (logf(maxX) - logf(margin));
//
//        labelGap = 20;
//        x = margin;
//        //loop through values
//        for(int i=startBin; i<endBin; i++){
//
//            y = -raw_fft[i]*maxHeight;
//
//            if(i==0) path.moveTo(logf(margin), y);
//            path.lineTo(logf(x), y);
//
//            if(label_inc >= labelGap){
//                label_inc = 0;
//                int freq = ((22050*i)/(raw_fft.size()));
//                freq -= (freq%100);
//                std::string label;
//                if(freq >= 1000){
//                    freq /= 1000.;
//                    std::stringstream stream;
//                    stream << std::fixed << std::setprecision(1) << freq;
//                    std::string s = stream.str();
//                    label = s+"k";
//                }
//                else {
//                    label = std::to_string(freq);
//                }
//
//                float lineX = margin+x_scale*(logf(x) - logf(margin));
//
//                float nextX = margin+x_scale*(logf(x+(x_inc*labelGap)) - logf(margin));
//
//
//                if((nextX-lineX) > labelGap) {
//                    ofSetColor(100);
//                    ofDrawBitmapString(label, lineX+3, 15);
//
//                }
//                ofSetColor(70);
//                ofDrawLine(lineX, 0, lineX, h);
//            }
//
//            x += x_inc;
//            label_inc += x_inc;
//            labelCount += 1;
//        }
//
//        utils::scalePath(&path, w*0.95, maxHeight);
//        ofRectangle bb = path.getOutline().front().getBoundingBox();
//        float px = bb.getX();
//        float py = bb.getY();
//
//        ofTranslate(margin-px, y_offset);
//        path.setFilled(false);
//        path.setStrokeColor(ofColor::white);
//        path.setStrokeWidth(1);
//
//        path.draw();
//
//        ofPopMatrix();
//    }
}


