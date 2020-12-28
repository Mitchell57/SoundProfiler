//
//  displayModes.cpp
//  soundProfiler
//
//  Created by Mitch on 12/27/20.
//

#include "DisplayMode.hpp"

DisplayMode::DisplayMode(){}

void DisplayMode::init(int w, int h){
    mode = LINEAR;
    updateLayout(w, h);
}

void DisplayMode::setMode(Mode m){
    mode = m;
}

DisplayMode::Mode DisplayMode::getMode(){
    return mode;
}

void DisplayMode::draw(Analysis analysis){
    // Draw graphs
    if(analysis.isFrameReady()){
        int scaleSize = analysis.getScaleSize();
        
        if(mode == LINEAR){
            int octSize = analysis.getOctaveSize();
            float* octave = analysis.getOctave();
            float* scale = analysis.getScale();
            
            ofPushMatrix();
            ofTranslate(singleXOffset,singleYOffset);
            drawLinOctave(singleW, singleH, octSize, octave);
            ofPopMatrix();
            
            ofPushMatrix();
            ofTranslate(multiXOffset, multiYOffset);
            drawLinScale(multiW, multiH, scaleSize, scale);
            ofPopMatrix();
        }
        if(mode == POLAR){
            float* raw_scale = analysis.getRawScale();
            
            ofPushMatrix();
            ofTranslate(0, 30);
            drawPolar(width, height-30, scaleSize, raw_scale);
            ofPopMatrix();
        }
    }
}

//--------------------------------------------------------------
void DisplayMode::drawLinOctave(int w, int h, int dataSize, float* data){
    
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
    ofPopStyle();
    
    // Initialize graph values
    //    Data = Summed Octave
    //    Bars take up 80% of total width
    //    Margins take up rest of total width
    //    Max bar height is 95% of height
    //    Labels are displayed if bar width > 12 pixels
    
    barWidth = (((float)w) * 0.8) / (float)dataSize;
    margin = (((float)w) - barWidth*dataSize) / ((float)dataSize+1);
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
    for(int i=0; i<dataSize; i++){
        
        ofPushStyle();
        
        ofRectangle rect;
        rect.x = x;
        rect.y = 0;
        rect.width = barWidth;

        // If rectangle height is below min. threshold, draw min rectangle
        // Note label follows rectangle if possible, otherwise sits on top of min rect.
        // y-axis is 'flipped' i.e. negative is upwards
        if(data[i] < 0.05 || data[i] > 1.0 || data[i] != data[i]) {
            rect.height =  -3;
            yPosLabel = -6;
        }
        else{
            rect.height = -data[i]*maxHeight;
            yPosLabel = std::max((int)rect.height-6, (int)-maxHeight-10);
        }
        
        
        float hue = (i%12)*(255.0/12);
        float sat = 100+data[i%12]*155;
        float brightness = 55+data[i]*200;
        
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
            octaveNum++;
        }
    }
    
    ofPopMatrix();
}

//--------------------------------------------------------------
void DisplayMode::drawLinScale(int w, int h, int dataSize, float* data){
    
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
    ofPopStyle();
    
    // Initialize graph values
    //    Data = Summed Octave
    //    Bars take up 80% of total width
    //    Margins take up rest of total width
    //    Max bar height is 95% of height
    //    No labels
    barWidth = (((float)w) * 0.8) / (float)dataSize;
    margin = (((float)w) - barWidth*dataSize) / ((float)dataSize+1);
    maxHeight = ((float)h)*0.95;
    y_offset = (float)(h + maxHeight)/2;
    
    int x = 0;
    int noteNum = 0;
    int octaveNum = 2;
    int labelXOffset = ((barWidth-15)/2, 0);
    
    ofPushMatrix();
    ofTranslate(margin, y_offset); //Move to bottom-left corner for start
    
    //loop through raw values
    for(int i=0; i<dataSize; i++){
        
        ofPushStyle();
        
        
        ofRectangle rect;
        rect.x = x;
        rect.y = 0;
        rect.width = barWidth;

        // If rectangle height is below min. threshold, draw min rectangle
        // y-axis is 'flipped' i.e. negative is upwards
        if(data[i] < 0.05 || data[i] > 1.0 || data[i] != data[i]) {
            rect.height =  -3;
        }
        else{
            rect.height = -data[i]*maxHeight;
        }
        
        float hue = (i%12)*(255.0/12);
        float sat = 100+data[i]*155;
        float brightness = 55+data[i]*200;
        

        
        ofColor color = ofColor::fromHsb(hue, sat, brightness);
        ofSetColor(color);
        
        ofDrawRectangle(rect);
        ofPopStyle();
        
        // increment x position, note, and octave (if necessary)
        x += barWidth+margin;
        noteNum++;
        if(noteNum > 11){
            noteNum = 0;
            octaveNum++;
        }
    }
    
    ofPopMatrix();
}

void DisplayMode::drawPolar(int w, int h, int dataSize, float* data){
    float constraint = min(width, height);
    float rMin = (constraint*0.1)/2;
    float rMax = (constraint*0.9)/2;
    
    float numOctaves = dataSize / 12;
    float r_inc = rMax / numOctaves;


    ofPushMatrix();
    ofTranslate(width/2, height/2);
    
    float deg = 0;
    float deg2;
    float rSmall = 0;
    float x1,y1,x2,y2,x3,y3,x4,y4;
    float rData, rad1, rad2, hue, sat, brightness;
    for(int i=0; i<dataSize; i++){
        if(i%12 == 0){
            rSmall += r_inc;
            deg = 0;
        }
        
        deg += 360/12;
        deg2 = deg+(360/12);
                
        if(data[i] < 0.1 || data[i] > 1 || data[i] != data[i]){
            rData = 0.1;
        }
        else{
            rData = data[i]*r_inc;
        }
        
        rad1 = ofDegToRad(deg);
        rad2 = ofDegToRad(deg2);
        
        x1 = rSmall * cos(rad1);
        y1 = rSmall * sin(rad1);
        
        x2 = (rSmall+rData) * cos(rad1);
        y2 = (rSmall+rData) * sin(rad1);
        
        x4 = rSmall * cos(rad2);
        y4 = rSmall * sin(rad2);
        
        x3 = (rSmall+rData) * cos(rad2);
        y3 = (rSmall+rData) * sin(rad2);
        

        
        hue = (i%12)*(255.0/12);
        sat = data[i]*255;
        brightness = data[i]*255;
        
        ofPath path;
        
        ofColor color = ofColor::fromHsb(hue, sat, brightness);
        path.setFillColor(color);
        
        path.moveTo(x1,y1);
        path.arc(0,0,rSmall, rSmall, deg, deg2);
        path.lineTo(x3, y3);
        path.arcNegative(0,0, (rSmall+rData), (rSmall+rData), deg2, deg);
        path.lineTo(x1,y1);
        
        path.close();
        path.setFilled(true);
        path.setStrokeHexColor(0);
        path.setStrokeWidth(1);
        path.draw();
        
        
        
        
    }
    
    ofPopMatrix();
}


void DisplayMode::updateLayout(int w, int h){
    width = w;
    height = (h-45);
    h -= 35;
    // Calculates new positions for graphs / controls when window is resized
    int topPadding = h*0.02;
    int lrPadding = w*0.02;
    
    singleW = ((float)w*0.45);
    singleH =  ((float)h*0.45);
    multiW = ((float)w*0.96);
    multiH =  ((float)h*0.46);
    singleXOffset = w - (2*lrPadding+singleW);
    singleYOffset = 45+topPadding;
    multiXOffset = lrPadding;
    multiYOffset = 45+(singleH+2*topPadding);
}

