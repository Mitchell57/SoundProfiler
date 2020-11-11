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
    //std::vector<int> binlist = freq2bin();
    //cp.init(fft->binlist);
    
    oct_size = chromaticScale.size();
    singleOctave = new float[oct_size];
    buffer = new float[oct_size];
    displayOctave = new float[oct_size];
    
    // Initialization for display values
    for(int i=0; i<oct_size; i++){
        displayOctave[i] = 0.01;
    }
    
    // Setup soundstream (default output / input channels)
    // On Mac:
    //      0 = current audio output device
    //      1 = current audio input device
    ofSoundStreamSetup(0, 1, this, 44100, bufferSize, 4);
    //ofSoundStreamStart();
    
    // Graph initialization
    //  bars take up 80% of total width
    //  margin between each bar and on either end
    //  max height is 95% of total height
    //  starting y position leaves a 2.5% margin on top/bottome
    float dataSize = 12.0;
    barWidth = (((float)WIN_WIDTH) * 0.8) / dataSize;
    margin = (((float)WIN_WIDTH) - barWidth*dataSize) / (dataSize+1);
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
    
    const std::lock_guard<std::mutex> lock(mtx);
    // Sum values per note across octaves
    float single_max = 0;
    for(int i=0; i<oct_size; i++){
        float val = 0;
        for(int j=-2; j<3; j+=1){
            val += fft->getAmplitudeAtFrequency(chromaticScale[i]*pow(2, j));
            
        }
        singleOctave[i] = val;
        if(val > single_max) single_max = val;
    }
    for(int i=0; i<oct_size; i++){
        singleOctave[i] /= single_max;
    }
    
    
    // Get amplitude bins from FFT
    //float* curFft = fft->getAmplitude();

}

//--------------------------------------------------------------
void ofApp::update(){
    const std::lock_guard<std::mutex> lock(mtx);
    memcpy(buffer, singleOctave, oct_size*sizeof(float));
    
}

//--------------------------------------------------------------
void ofApp::exit(){
    //ofSoundStreamStop();
    //ofSoundStreamClose();
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    // Update display data
    for(int i=0; i<oct_size; i++){
        float diff = buffer[i] - displayOctave[i];
        if(diff > 0){
            if(buffer[i] >= displayOctave[i]*1.2){
                displayOctave[i] *= 1.2;
            }
            else{
                displayOctave[i] = buffer[i];
            }
        }
        else{
            if(buffer[i] <= displayOctave[i]*0.5){
                displayOctave[i] *= 0.5;
            }
            else{
                displayOctave[i] = buffer[i];
            }
        }
    }
    
    if(displayOctave[0] != displayOctave[0]){
        for(int i=0; i<oct_size; i++){
            displayOctave[i] = 0.01;
        }
    }
    /*for(int i=0; i<oct_size; i++){
        displayOctave[i] = singleOctave[i];
    }*/
    
    
    // Initialize graph values
    int x = 0;
    int noteNum = 0;
    int octaveNum = 0;
    int labelXOffset = max((barWidth-15)/2, 0);
    int yPos;
    
    ofPushMatrix();
    ofTranslate(margin, y_offset); //Move to bottom-left corner for start
    
    //loop through raw values
    for(int i=0; i<oct_size; i++){
        
        ofPushStyle();
        ofSetColor(colors[noteNum]);
        
        ofRectangle rect;
        rect.x = x;
        rect.y = 0;
        rect.width = barWidth;

        // If rectangle height is below min. threshold, draw min rectangle
        // Note label follows rectangle if possible, otherwise sits on top of min rect.
        // y-axis is 'flipped' i.e. negative is upwards
        if(displayOctave[i] < 0.05 || displayOctave[i] != displayOctave[i]) {
            rect.height =  -3;
            yPos = -6;
        }
        else{
            rect.height = -displayOctave[i]*maxHeight;
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


std::vector<int> ofApp::freq2bin(){
    
    // fundamental frequencies of A4-G#4
    std::vector<float> chromaticScale = {440, 466.16, 493.88, 523.25, 554.37, 587.33, 622.25, 659.26, 698.46, 739.99, 783.99, 830.61};
    std::vector<float> freqlist;
    std::vector<int> binlist;
    
    // build list of frequencies from A2-G#7
    for(int i=-2; i<=3; i++){
        for(int j=0; j<chromaticScale.size(); j++){
            freqlist.push_back(chromaticScale[j]*pow(2, i));
        }
    }
    
    // Translate frequency list to bin list
    for(int i=0; i<freqlist.size(); i++){
        float bin = fft->getBinFromFrequency(freqlist[i], 44100);
        binlist.push_back((int)bin);
    }
    
    return binlist;
}

