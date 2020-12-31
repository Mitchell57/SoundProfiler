//
//  OscDisplay.cpp
//  SoundProfiler
//
//  Created by Mitch on 12/29/20.
//

#include <stdio.h>
#include "OscDisplay.h"

OscDisplay::OscDisplay(){
    name = "Nebula";
}

void OscDisplay::setup(){
    dataRequest = {utils::SMOOTH_SCALE, utils::RAW_SCALE };
    
    timer = 0;
    dataSize = 1;
    
    
    blur.setup(600,600, 30, .2, 2);
    blur2.setup(600,600, 25, .2, 2);
}

void OscDisplay::buildGui(ofxGuiGroup *parent){
    group = parent->addGroup("osc parameters");
    group->setShowHeader(false);
    
    globalGroup = group->addGroup("Global Controls");
    globalGroup->add(colorWidth.set("Hue Width", 157, 0, 255));
    globalGroup->add(colorShift.set("Hue Shift", 133, 0, 255));
//    globalGroup->add(smooth.set("Smooth", 3.5, 1., 5.));
    globalGroup->add<ofxGuiFloatSlider>(smooth.set("Smooth", 3.5, 1., 5.), ofJson({{"precision", 2}}));
    
    oscGroup = group->addGroup("Dot Controls");
    oscGroup->add(oscColorShift.set("Hue Shift", 50, 0, 255));
    oscGroup->add(speed.set("Speed", 1.6, 0.1, 10.));

    
    group->add(parameters);
}


void OscDisplay::draw(){
    ofPushStyle();
    ofPushMatrix();
    
    drawOscillator(width, height);
    
    blur2.begin();
    ofClear(0, 0, 0, 10);
    drawPolar(width, height);
    blur2.end();
    blur2.draw();
    
    ofPopMatrix();
    ofPopStyle();
}


void OscDisplay::update(std::vector<utils::soundData> newData){
    for(utils::soundData container : newData){
        if(dataSize != container.data.size()) {
            dataSize = container.data.size();
            scale.resize(dataSize);
            raw_scale.resize(dataSize);
            xVals.resize(dataSize);
            yVals.resize(dataSize);
            rVals.resize(dataSize);
        }
        
        switch (container.label) {
            case utils::SMOOTH_SCALE:
            case utils::SMOOTH_SCALE_OT:
                for(int i=0; i<scale.size(); i++){
                    scale[i] = utils::approxRollingAverage(scale[i], container.data[i], (int)(smooth));
                }
                break;
                
            case utils::RAW_SCALE:
                for(int i=0; i<scale.size(); i++){
                    raw_scale[i] = utils::approxRollingAverage(raw_scale[i], container.data[i], (int)(smooth));
                }
                break;
            default:
                break;
        }
    }
    
}
void OscDisplay::setDimensions(int w, int h){
    width = w;
    height = h;
    
    blur.setup(w, h, 30, .2, 2);
    blur2.setup(w, h, 25, .2, 2);
}


//--------------------------------------------------------------------------------------
// polar
//--------------------------------------------------------------------------------------
void OscDisplay::drawPolar(int w, int h){
    if(scale.size() <= 1) return;
    
    float constraint = max(width, height);
    float rMin = (constraint*0.1)/2;
    float rMax = (constraint*0.9)/2;
    
    float numOctaves = scale.size() / 12;
    float r_inc = rMax / numOctaves;


    ofPushMatrix();
    ofTranslate(width/2, height/2);
    
    float deg = 0;
    float deg2;
    float rSmall = 0;
    float x1,y1,x2,y2,x3,y3,x4,y4;
    float rData, rad1, rad2, hue, sat, brightness;
    for(int i=0; i<scale.size(); i++){
        if(i%12 == 0){
            rSmall += r_inc;
            deg = 0;
        }
        
        deg += 360/12;
        deg2 = deg+(360/12);
                
        if(scale[i] > 1 || scale[i] != scale[i]){
            rData = 0;
        }
        else{
            rData = scale[i]*r_inc;
        
        
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
        

        
        //hue = (i%dataSize)*(255.0/dataSize);
        hue = (colorShift+(((float)i/scale.size())*colorWidth));
        hue = ((int)hue)%255;

        sat = 100+scale[i]*155;
        brightness = 90+scale[i]*165;
        float alpha = min((float)255.0, (40+260*scale[i]));
        if(scale[i] < 0.15) {
            brightness = scale[i]*255;
            alpha = scale[i]*255;
        }
        
            
        ofPath path;
        ofSetCurveResolution(100);
        ofColor color = ofColor::fromHsb(hue, sat, brightness);
        path.setFillColor(ofColor(color, alpha));
        
        path.moveTo(x1,y1);
        path.arc(0,0,rSmall, rSmall, deg, deg2);
        path.lineTo(x3, y3);
        path.arcNegative(0,0, (rSmall+rData), (rSmall+rData), deg2, deg);
        path.lineTo(x1,y1);
        
        path.close();
        path.setFilled(true);
        path.setStrokeHexColor(0);
        path.setStrokeWidth(0);
        path.draw();
        }
    }
    ofPopMatrix();
}


//--------------------------------------------------------------------------------------
// oscillator
//--------------------------------------------------------------------------------------
void OscDisplay::drawOscillator(float w, float h){
    if(scale.size() <= 1) return;

    float constraint = max(w, h);
    float maxR = (constraint*0.9)/2;
    float minR = (constraint*0.05)/2;

    float theta = (timer)/7;
    float hue, sat, brightness, radius;
    blur.begin();
    ofClear(0, 0, 0, 3);
    ofPushMatrix();
    ofTranslate(w/2, h/2);
    
    float newsum = 0;
    for(int i=0; i<dataSize; i++){
        newsum += scale[i];
    }
    newsum /= dataSize;
    
    sum = utils::approxRollingAverage(sum, newsum, 15);
    
    timer += sum*speed;
    
    for(int i=0; i<dataSize; i++){
        radius = minR+(maxR-minR)*(scale[i]+3*sum)/2;
        
        hue = (oscColorShift+colorShift+(((float)i/dataSize)*colorWidth));
        hue = ((int)hue)%255;
        sat = ((100)+(155.0*scale[i]));
        brightness = ((255)-(95.0*scale[i]))*(sum*2.8);
        
        
        ofColor color = ofColor::fromHsb(hue, sat, brightness);
        ofSetColor(color);
        
        float n = 3;
        
        float x = radius*cos((i+theta+scale[i])/2);
        xVals[i] -= xVals[i]/n;
        xVals[i] += x/n;
        
        float y = radius*sin((i+theta+scale[i])/3);
        yVals[i] -= yVals[i]/n;
        yVals[i] += y/n;
        
        float r = (radius/2)*(sum+scale[i])/2;
        rVals[i] = max(utils::approxRollingAverage(rVals[i], r, n), (float)0.001);
        
        ofDrawCircle(xVals[i], yVals[i], rVals[i]);
    }
    
    
    ofPopMatrix();
    blur.end();
    blur.draw();
}
