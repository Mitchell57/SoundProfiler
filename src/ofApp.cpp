#include "ofApp.h"
#include <string>
#include <algorithm>    // std::max

//--------------------------------------------------------------
void ofApp::setup(){
   
    // 0 output channels,
    // 1 input channel
    // 44100 samples per second
    // 4096 samples per buffer
    // 4 num buffers (latency)
    bufferSize = 4096;
    
    // Create FFT and ChromaticParse objects
    fft = ofxFft::create(bufferSize, OF_FFT_WINDOW_BARTLETT);
    cp.init(fft->getBinSize());
    
    // Setup soundstream (default output / input channels)
    // On Mac:
    //      0 = current audio output device
    //      1 = current audio input device
    ofSoundStreamSetup(0, 1, this, 44100, bufferSize, 4);
    
    // Graph initialization
    //  bars take up 80% of total width
    //  margin between each bar and on either end
    //  max height is 95% of total height
    //  starting y position leaves a 2.5% margin on top/bottome
    barWidth = (((float)WIN_WIDTH) * 0.8) / 12.0;
    margin = (((float)WIN_WIDTH) - barWidth*12.0) / (12.0+1);
    maxHeight = ((float)WIN_HEIGHT)*0.95;
    y_offset = (float)(WIN_HEIGHT + maxHeight)/2;
    
    // Dark grey background
    ofBackground(40);
}

//--------------------------------------------------------------
void ofApp::audioReceived(float* input, int bufferSize, int nChannels) {
    
    // Scale audio input frame to {-1, 1}
    float maxValue = 0;
    for(int i = 0; i < bufferSize; i++) {
        if(abs(input[i]) > maxValue) {
            maxValue = abs(input[i]);
        }
    }
    for(int i = 0; i < bufferSize; i++) {
        input[i] = input[i] / maxValue;
    }
    
    // Send scaled frame to FFT
    fft->setSignal(input);
    
    // Get amplitude bins from FFT
    float* curFft = fft->getAmplitude();
    
    // Send FFT data to ChromaticParse object
    cp.updateData(curFft);
}

//--------------------------------------------------------------
void ofApp::update(){
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    // Get chromatic data
    std::vector<float> data = cp.getOctave();
    
    // Initialize graph values
    int x = 0;
    int noteNum = 0;
    int octaveNum = 0;
    int labelXOffset = max((barWidth-15)/2, 0);
    int yPos;
    
    ofPushMatrix();
    ofTranslate(margin, y_offset); //Move to bottom-left corner for start
    
    //loop through raw values
    for(int i=0; i<data.size(); i++){
        
        ofPushStyle();
        ofSetColor(colors[noteNum]);
        
        ofRectangle rect;
        rect.x = x;
        rect.y = 0;
        rect.width = barWidth;

        // If rectangle height is below min. threshold, draw min rectangle
        // Note label follows rectangle if possible, otherwise sits on top of min rect.
        // y-axis is 'flipped' i.e. negative is upwards
        if(data[i] < 0.05) {
            rect.height =  -3;
            yPos = -6;
        }
        else{
            rect.height = -data[i]*maxHeight;
            yPos = rect.height+15;
        }
        
        ofDrawRectangle(rect);
        ofPopStyle();
        
        // Draw note label
        ofSetColor(ofColor::white);
        std::string label = noteNames[noteNum];
        ofDrawBitmapString(label, x+labelXOffset, yPos);
        
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
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}


//--------------------------------------------------------------
vector<int> polar2cartesian(int r, float theta, float center){
    vector<int> coords;
    coords.push_back(round(r*cos(theta)+center));
    coords.push_back(round(r*sin(theta)+center));
    return coords;
}

