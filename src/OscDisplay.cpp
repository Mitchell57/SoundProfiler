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
    
    
    parameters.setName("Oscillator Controls");
    parameters.add(colorWidth.set("Color Width", 120, 0, 255));
    parameters.add(colorShift.set("Color Shift", 0, 0, 255));
    parameters.add(smooth.set("Smooth", 0.25, 0., 1.));

    
    dataRequest = {utils::SMOOTH_SCALE, utils::RAW_SCALE };
    
    osc_data1 = new float[scale.size()];
    osc_data2 = new float[scale.size()];
    xVals = new float[scale.size()];
    yVals = new float[scale.size()];
    rVals = new float[scale.size()];
    
    for(int i=0; i<scale.size(); i++){
        osc_data1[i] = 0.1;
        osc_data2[i] = 0.1;
        xVals[i] = 0.1;
        yVals[i] = 0.1;
        rVals[i] = 0.1;
    }
}


void OscDisplay::draw(){
    ofPushMatrix();

    drawOscillator(width, height);
    
    blur2.begin();
    ofClear(0, 0, 0, 10);
    drawPolar(width, height);
    blur2.end();
    blur2.draw();
    
    ofPopMatrix();
}


void OscDisplay::update(std::vector<utils::soundData> newData){

    for(utils::soundData container : newData){
        switch (container.label) {
            case utils::SMOOTH_SCALE:
            case utils::SMOOTH_SCALE_OT:
                scale = container.data;
                break;
                
            case utils::RAW_SCALE:
                raw_scale = container.data;
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
    
    float constraint = min(width, height);
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
        float alpha = min((float)255.0, (40+280*scale[i]));
        if(scale[i] < 0.15) brightness = scale[i]*255;
        
            
        ofPath path;
        ofSetCurveResolution(100);
        ofColor color = ofColor::fromHsb(hue, sat, brightness);
        path.setFillColor(ofColor(color, brightness));
        
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
void OscDisplay::drawOscillator(int w, int h){
    if(scale.size() <= 1) return;

    for(int i=0; i<scale.size(); i++){
        float val1 = utils::approxRollingAverage(osc_data1[i], scale[i], 20);
        float val2 = utils::approxRollingAverage(osc_data2[i], raw_scale[i], 6);
        
        osc_data1[i] += (val1-osc_data1[i])*smooth;
        osc_data2[i] += (val2-osc_data2[i])*smooth;
    }
    float constraint = min(w, h);
    float maxR = (constraint*0.9)/2;
    float minR = (constraint*0.05)/2;

    float theta = (timer)/7;
    float hue, sat, brightness, radius;
    blur.begin();
    ofClear(0, 0, 0, 3);
    ofPushMatrix();
    ofTranslate(w/2, h/2);
    
    float newsum = 0;
    for(int i=0; i<scale.size(); i++){
        newsum += osc_data1[i];
    }
    newsum /= scale.size();
    
    sum = utils::approxRollingAverage(sum, newsum, 15);
    
    timer += sum/1.75;
    
    for(int i=0; i<scale.size(); i++){
        radius = minR+(maxR-minR)*(osc_data1[i]+3*sum)/2;
        
        hue = (colorShift+(((float)i/scale.size())*colorWidth));
        hue = ((int)hue)%255;
        sat = ((100)+(155.0*osc_data1[i]));
        brightness = ((255)-(95.0*osc_data1[i]));//*(sum*2.8);
        
        ofColor color = ofColor::fromHsb(hue, sat, brightness);
        ofSetColor(color);
        
        float n = 3;
        
        float x = radius*cos((i+theta+osc_data1[i])/2);
        xVals[i] -= xVals[i]/n;
        xVals[i] += x/n;
        
        float y = radius*sin((i+theta+osc_data2[i])/3);
        yVals[i] -= yVals[i]/n;
        yVals[i] += y/n;
        
        float r = (radius/2)*(sum+osc_data1[i])/2;
        rVals[i] -= rVals[i]/n;
        rVals[i] += r/n;
        
        ofDrawCircle(xVals[i], yVals[i], rVals[i]);
    }
    
    ofPopMatrix();
    blur.end();
    blur.draw();
}
