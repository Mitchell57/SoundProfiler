#include "ofApp.h"
#include <string>
#include <algorithm>    // std::max

//--------------------------------------------------------------
void ofApp::setup(){
   
    ofSetFrameRate(120);
    inputBool = true;
    shouldFactorAgg = true;
    
    bufferSize = 2048;
    file.openFile(ofToDataPath("flamenco-sketches.wav",true));
    fft = ofxFft::create(bufferSize, OF_FFT_WINDOW_BARTLETT);
    
    // data array initialization
    // build list of frequencies from A2-G#7
    for(int i=-2; i<=3; i++){
        for(int j=0; j<chromaticScale.size(); j++){
            freqlist.push_back(chromaticScale[j]*pow(2, i));
        }
    }
    fullBinList = freq2bin();
    
    oct_size = chromaticScale.size();
    scale_size = fullBinList.size();
    wholeDataSize = oct_size+scale_size;
    
    audioData = new float[wholeDataSize];
    buffer = new float[wholeDataSize];
    displayData = new float[wholeDataSize];
    
    // Initialization for display values
    for(int i=0; i<wholeDataSize; i++){
        displayData[i] = 0.01;
    }
    
    // Setup soundstream (default output / input channels)
    // 2 output channels,
    // 1 input channel
    // 44100 samples per second
    // 4096 samples per buffer
    // 4 num buffers (latency)
    
    stk::Stk::setSampleRate(44100.0);
    ofSoundStreamSettings settings;
    settings.setOutListener(this);
    settings.setInListener(this);
    settings.numOutputChannels = 2;
    settings.numInputChannels = 1;
    settings.numBuffers = 9;
    settings.bufferSize = bufferSize;
    soundStream.setup(settings);
    
    
    
    
    
    // Dark grey background
    ofBackground(40);
    
    
    
    
    
    inputToggle.addListener(this, &ofApp::inputPressed);
    outputToggle.addListener(this, &ofApp::outputPressed);
    factorToggle.addListener(this, &ofApp::factorAggPressed);
    panel = gui.addGroup("Chromatic Profiler");
    //panel = gui.addPanel("Chromatic Profiler");
    panel->setPosition(10,10);
    panel->setDraggable(false);
    audioModes = panel->addGroup("Audio Mode");
    audioModes->add(inputToggle.set("Stream Audio", true));
    audioModes->add(outputToggle.set("Play File", false));
    panel->addSpacer(0,20);
    graphControls = panel->addGroup("Graph Controls");
    graphControls->add(factorToggle.set("Factor Aggregate", true));
    panel->addSpacer(0,20);
    panel->add(loadButton.set("Load File"));
    panel->add(playButton.set("Play File"));
    
    updateLayout(WIN_WIDTH, WIN_HEIGHT);
}

//--------------------------------------------------------------
void ofApp::inputPressed(bool &inputToggle){
    //this->inputToggle.set(true);
    
    if(!inputBool && inputToggle){
        inputBool = true;
        outputToggle.set(false);
        clearGraphs();
    }

    
}

//--------------------------------------------------------------
void ofApp::outputPressed(bool &outputToggle){
    //this->outputToggle.set(true);
    
    if(inputBool && outputToggle){
        inputBool = false;
        inputToggle.set(false);
        clearGraphs();
    }
    
}

//--------------------------------------------------------------
void ofApp::factorAggPressed(bool &factorToggle){
    shouldFactorAgg = factorToggle;
}

//--------------------------------------------------------------
void ofApp::audioIn(ofSoundBuffer& buffer) {
    if(inputBool){
        auto& input = buffer.getBuffer();
        auto bufferSize = buffer.getNumFrames();
        auto numChannels = buffer.getNumChannels();
        stk::StkFrames frames(bufferSize,numChannels);
        
        if(numChannels > 1){
            stk::StkFrames leftChannel(bufferSize,1);
            frames.getChannel(0, leftChannel, 0);
            for (int i = 0; i < bufferSize ; i++) {
                input[2*i] = leftChannel(i,0);
                input[2*i+1] = leftChannel(i,0);
            }
        }

        analyzeAudio(input, bufferSize);
        
    }
}

//--------------------------------------------------------------
void ofApp::audioOut(ofSoundBuffer& buffer){
    if( !inputBool ){
        auto& output = buffer.getBuffer();
        auto bufferSize = buffer.getNumFrames();
        if (shouldPlayAudio) {
            stk::StkFrames frames(bufferSize,2);
            file.tick(frames);
            
            // the file is usually 2 channels , however we only want one
            // so we will just use the left channel.
            stk::StkFrames leftChannel(bufferSize,1);
            // copy the left Channel of 'frames' into `leftChannel`
            frames.getChannel(0, leftChannel, 0);
            for (int i = 0; i < bufferSize ; i++) {
                output[2*i] = leftChannel(i,0);
                output[2*i+1] = leftChannel(i,0);
            }
        }

        analyzeAudio(output, (int)bufferSize);
        //printf("max: %f\n", outputTotal);
    }
}

//--------------------------------------------------------------
void ofApp::analyzeAudio(std::vector<float> sample, int bufferSize){
    // Scale audio input frame to {-1, 1}
    float maxValue = 0;
    float* normalizedOut = new float[bufferSize];
    for(int i = 0; i < bufferSize; i+=2) {
        if(abs((float)sample[i]) > maxValue) {
            maxValue = abs(sample[i]);
        }
        
    }
    for(int i = 0; i < bufferSize; i++) {
        normalizedOut[i] = (float)sample[i] / maxValue;
    }
    
    // Send scaled frame to FFT
    fft->setSignal(sample);
    
    // audio listeners run in a separate thread
    //   so we must ensure control before making changes to
    //   a shared variable
    const std::lock_guard<std::mutex> lock(mtx);
    
    // Sum values per note across octaves
    float single_max = 0;
    float scale_max = 0;
    for(int i=0; i<oct_size; i++){
        float val = 0;
        for(int j=-2; j<3; j+=1){
            val += fft->getAmplitudeAtFrequency(chromaticScale[i]*pow(2, j));
            
        }
        audioData[i] = val;
        if(val > single_max) single_max = val;
    }
    for(int i=0; i<oct_size; i++){
        audioData[i] /= single_max;
    }
    
    for(int i=oct_size; i<wholeDataSize; i++){
        float val = fft->getAmplitudeAtBin(fullBinList[i-oct_size]);
        if(val > scale_max) scale_max = val;
        audioData[i] = val;
    }
    for(int i=oct_size; i<wholeDataSize; i++){
        audioData[i] /= scale_max;
    }
}

//--------------------------------------------------------------
void ofApp::clearGraphs(){
    for(int i=0; i<wholeDataSize; i++){
        displayData[i] = 0.02;
    }
}

//--------------------------------------------------------------
void ofApp::update(){
    // audio listeners run in a separate thread
    //   so we must ensure control before making changes to
    //   a shared variable
    const std::lock_guard<std::mutex> lock(mtx);
    
    // copy octave data to buffer
    memcpy(buffer, audioData, wholeDataSize*sizeof(float));
    
}

//--------------------------------------------------------------
void ofApp::exit(){

}

//--------------------------------------------------------------
void ofApp::draw(){
    
    // NaN checker
    // (doesn't get used often but sometimes it's helpful
    if(buffer[0] != buffer[0]){
        // NaN. skip frame
        if(!inputBool) clearGraphs();
    }
    else{
        for(int i=0; i<wholeDataSize; i++){
            if(shouldFactorAgg && i >= oct_size) {
                displayData[i] = (buffer[i] + 2*displayData[i] + displayData[i%oct_size])/4.0;
            }
            else displayData[i] = (buffer[i] + displayData[i])/2;
            displayData[i] *= displayData[i];
        }
    }
    
    ofPushMatrix();
    ofTranslate(singleXOffset,singleYOffset);
    drawSingleOctave(singleW, singleH);
    ofPopMatrix();
    
    ofPushMatrix();
    ofTranslate(multiXOffset, multiYOffset);
    drawMultiOctave(multiW, multiH);
    ofPopMatrix();
}

//--------------------------------------------------------------
void ofApp::drawSingleOctave(float width, float height){
    
    ofPushStyle();
    ofSetColor(ofColor::white);
    ofNoFill();
    ofRectangle outer_rect;
    outer_rect.x = 0;
    outer_rect.y = 0;
    outer_rect.width = width;
    outer_rect.height = height;
    ofDrawRectangle(outer_rect);
    ofPopStyle();
    
    // Initialize graph values
    int dataSize = 12;
    barWidth = (((float)width) * 0.8) / (float)dataSize;
    bool labelsOn = (barWidth > 12);
    margin = (((float)width) - barWidth*dataSize) / ((float)dataSize+1);
    maxHeight = ((float)height)*0.95;
    y_offset = (float)(height + maxHeight)/2;
    int x = 0;
    int noteNum = 0;
    int octaveNum = 0;
    int labelXOffset = max((barWidth-15)/2, 0);
    int yPos;
    
    ofPushMatrix();
    ofTranslate(margin, y_offset); //Move to bottom-left corner for start
    
    //loop through raw values
    for(int i=0; i<dataSize; i++){
        
        ofPushStyle();
        ofSetColor(colors[noteNum]);
        
        ofRectangle rect;
        rect.x = x;
        rect.y = 0;
        rect.width = barWidth;

        // If rectangle height is below min. threshold, draw min rectangle
        // Note label follows rectangle if possible, otherwise sits on top of min rect.
        // y-axis is 'flipped' i.e. negative is upwards
        if(displayData[i] < 0.05 || displayData[i] != displayData[i]) {
            rect.height =  -3;
            yPos = -6;
        }
        else{
            rect.height = -displayData[i]*maxHeight;
            yPos = std::max((int)rect.height-6, (int)-maxHeight-10);
        }
        
        ofDrawRectangle(rect);
        ofPopStyle();
        
        // Draw note label
        if(labelsOn){
            ofSetColor(ofColor::white);
            std::string label = noteNames[noteNum];
            ofDrawBitmapString(label, x+labelXOffset, yPos);
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
void ofApp::drawMultiOctave(float width, float height){
    
    ofPushStyle();
    ofSetColor(ofColor::white);
    ofNoFill();
    ofRectangle outer_rect;
    outer_rect.x = 0;
    outer_rect.y = 0;
    outer_rect.width = width;
    outer_rect.height = height;
    ofDrawRectangle(outer_rect);
    ofPopStyle();
    
    // Initialize graph values
    int dataSize = scale_size;
    barWidth = (((float)width) * 0.8) / (float)dataSize;
    bool labelsOn = (barWidth > 12);
    margin = (((float)width) - barWidth*dataSize) / ((float)dataSize+1);
    maxHeight = ((float)height)*0.95;
    y_offset = (float)(height + maxHeight)/2;
    int x = 0;
    int noteNum = 0;
    int octaveNum = 2;
    int labelXOffset = max((barWidth-15)/2, 0);
    int yPos;
    
    ofPushMatrix();
    ofTranslate(margin, y_offset); //Move to bottom-left corner for start
    
    //loop through raw values
    for(int i=oct_size; i<wholeDataSize; i++){
        
        ofPushStyle();
        ofSetColor(colors[noteNum]);
        
        ofRectangle rect;
        rect.x = x;
        rect.y = 0;
        rect.width = barWidth;

        // If rectangle height is below min. threshold, draw min rectangle
        // Note label follows rectangle if possible, otherwise sits on top of min rect.
        // y-axis is 'flipped' i.e. negative is upwards
        if(displayData[i] < 0.05 || displayData[i] != displayData[i]) {
            rect.height =  -3;
            yPos = -6;
        }
        else{
            rect.height = -displayData[i]*maxHeight;
            yPos = std::max((int)rect.height-6, (int)-maxHeight-10);
        }
        
        ofDrawRectangle(rect);
        ofPopStyle();
        
        // Draw note label
        if(labelsOn){
            ofSetColor(ofColor::white);
            std::string label = noteNames[noteNum]+"\n"+to_string(octaveNum);
            ofDrawBitmapString(label, x+labelXOffset, yPos);
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
void ofApp::keyPressed(int key){
    if (key == ' ') {
        shouldPlayAudio = !shouldPlayAudio;
    }
    if (key == 'm'){
        inputBool = !inputBool;
    }
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
    updateLayout(w, h);
}

void ofApp::updateLayout(int w, int h){
    int topPadding = h*0.03;
    int lrPadding = w*0.02;
    panel->setPosition(lrPadding, topPadding);
    
    singleW = ((float)w*0.45);
    singleH =  ((float)h*0.45);
    multiW = ((float)w*0.96);
    multiH =  ((float)h*0.46);
    singleXOffset = w - (2*lrPadding+singleW);
    singleYOffset = topPadding;
    multiXOffset = lrPadding;
    multiYOffset = (singleH+2*topPadding);
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}


//--------------------------------------------------------------
std::vector<int> ofApp::freq2bin(){
    
    // vector initialization
    std::vector<int> binlist;
    
    // Translate frequency list to bin list
    for(int i=0; i<freqlist.size(); i++){
        float bin = fft->getBinFromFrequency(freqlist[i], 44100);
        binlist.push_back((int)bin);
        cout << "bin: " << bin << "freq: " << freqlist[i] << endl;
    }
    
    return binlist;
}

