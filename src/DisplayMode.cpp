//
//  displayModes.cpp
//  soundProfiler
//
//  Created by Mitch on 12/27/20.
//

#include "DisplayMode.hpp"

// Approximate Rolling Average from:
// https://bit.ly/3aIsQRD
float approxRollingAverage (float avg, float new_sample, float n) {

    avg -= avg / n;
    avg += new_sample / n;

    return avg;
}

DisplayMode::DisplayMode(){}

void DisplayMode::init(int w, int h){
    mode = utils::LINEAR;
    updateLayout(w, h);
    
    fftLinear = false;
    timer = 0;
    
    sum = 0;
    smooth = 0.25;
    colorWidth = 120;
    colorShift = 0;
    
    osc_started = false;
    
    
}

//--------------------------------------------------------------------------------------
// main draw (per-frame entry point)
//--------------------------------------------------------------------------------------
void DisplayMode::draw(Analysis analysis){

    if(analysis.isFrameReady()){
        if(mode == utils::LINEAR){
            
        }
        if(mode == utils::RAW){
            float* raw_fft = analysis.getFft();
            int fft_size = analysis.getFftSize();
            
            ofPushMatrix();
            ofTranslate(xOffset, yOffset);
            drawFftPlot(halfW, halfH, fft_size, raw_fft);
            ofPopMatrix();
        }
        if(mode == utils::OSC){
            float* scale = analysis.getScale();
            float* raw_scale = analysis.getRawScale();
            int scale_size = analysis.getScaleSize();
            
            ofPushMatrix();
    
            drawOscillator(width, height, scale_size, scale, scale);
            
            blur2.begin();
            ofClear(0, 0, 0, 10);
            drawPolar(width, height, scale_size, scale);
            blur2.end();
            blur2.draw();
            
            ofPopMatrix();
            
        }
    }
    
    // Frame not available, draw empty boxes instead
    else{
        if(mode == utils::LINEAR){
            ofPushMatrix();
            ofTranslate(xOffset,yOffset);
            drawLinOctave(halfW, halfH, 1, 0);
            ofPopMatrix();
            
            ofPushMatrix();
            ofTranslate(xOffset, halfH+(2*yOffset));
            drawLinScale(halfW, halfH, 1, 0);
            ofPopMatrix();
        }
        if(mode == utils::POLAR){
            ofPushMatrix();
            drawPolar(width, height, 1, 0);
            ofPopMatrix();
        }
        if(mode == utils::RAW){
            ofPushMatrix();
            ofTranslate(xOffset, yOffset);
            drawFftPlot(halfW, halfH, 1, 0);
            ofPopMatrix();
            
        }
    }
}




//--------------------------------------------------------------------------------------
// fft plot
//--------------------------------------------------------------------------------------
void DisplayMode::drawFftPlot(int w, int h, int dataSize, float* data){
    
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
    
    if(dataSize <= 1) return;
    dataSize *= 0.75;
    float x = 0;
    float y = 0;
    
    float x_inc = (((float)w) * 0.99) / (float)dataSize;
    margin = (((float)w) - x_inc*dataSize) / 2;
    maxHeight = ((float)h)*0.95;
    y_offset = (float)(h + maxHeight)/2;
    
    ofSetColor(255);
    ofPath path;
    
    if(fftLinear){
        ofPushMatrix();
        ofTranslate(0, y_offset); //Move to bottom-left corner for start
        
        path.moveTo(0, -data[0]*maxHeight);
        
        //loop through values
        for(int i=1; i<dataSize; i++){
            x += x_inc;
            y = -data[i]*maxHeight;
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
        
        
        path.moveTo(log2f(margin), -data[0]*maxHeight);
        x = margin;
        float max_y = 0;
        //loop through values
        for(int i=1; i<dataSize; i++){
            x += x_inc;
            y = -data[i]*maxHeight;
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


//--------------------------------------------------------------------------------------
// polar
//--------------------------------------------------------------------------------------
void DisplayMode::drawPolar(int w, int h, int dataSize, float* data){
    if(dataSize <= 1) return;
    
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
                
        if(data[i] > 1 || data[i] != data[i]){
            rData = 0;
        }
        else{
            rData = data[i]*r_inc;
        
        
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
        hue = (colorShift+(((float)i/dataSize)*colorWidth));
        hue = ((int)hue)%255;

        sat = 100+data[i]*155;
        brightness = 90+data[i]*165;
        float alpha = min((float)255.0, (40+280*data[i]));
        if(data[i] < 0.15) brightness = data[i]*255;
        
            
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
void DisplayMode::drawOscillator(int w, int h, int dataSize, float* data, float* data2){
    if(dataSize <= 1) return;
    if(!osc_started){
        osc_data1 = data;
        osc_data2 = data2;
        
        xVals = new float[dataSize];
        yVals = new float[dataSize];
        rVals = new float[dataSize];
        
        for(int i=0; i<dataSize; i++){
            xVals[i] = 0.1;
            yVals[i] = 0.1;
            rVals[i] = 0.1;
        }
        osc_started = true;
    }
    else{
        for(int i=0; i<dataSize; i++){
            float val1 = approxRollingAverage(osc_data1[i], data[i], 20);
            float val2 = approxRollingAverage(osc_data2[i], data2[i], 6);
            
            osc_data1[i] += (val1-osc_data1[i])*smooth;
            osc_data2[i] += (val2-osc_data2[i])*smooth;
        }
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
    for(int i=0; i<dataSize; i++){
        newsum += osc_data1[i];
    }
    newsum /= dataSize;
    
    sum -= sum/15;
    sum += newsum/15;
    
    timer += sum/1.75;
    
    for(int i=0; i<dataSize; i++){
        radius = minR+(maxR-minR)*(osc_data1[i]+3*sum)/2;
        
        hue = (colorShift+(((float)i/dataSize)*colorWidth));
        hue = ((int)hue)%255;
        sat = ((100)+(155.0*osc_data1[i]));
        brightness = ((255)-(95.0*osc_data1[i]))*(sum*2.8);
        
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


//--------------------------------------------------------------
void DisplayMode::updateLayout(int w, int h){
    width = w;
    height = h;
    blur.setup(w, h, 30, .2, 2);
    blur2.setup(w, h, 25, .2, 2);
    
    halfW = ((float)w*0.9);
    halfH =  ((float)h*0.425);

    xOffset = h*0.05;
    yOffset = w*0.05;
}


//--------------------------------------------------------------------------------
//   getters & setters
//--------------------------------------------------------------------------------
void DisplayMode::setColorWidth(int val){ colorWidth = val; }

void DisplayMode::setColorShift(int val){ colorShift = val; }

void DisplayMode::setSmooth(float val){ smooth = val; }

void DisplayMode::setMode(utils::Mode m){ mode = m; }

utils::Mode DisplayMode::getMode(){ return mode; }

