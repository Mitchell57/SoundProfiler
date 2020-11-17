#include "ofApp.h"
#include <string>
#include <algorithm>    // std::max

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetFrameRate(30);
    ofBackground(40);
    
    // Boolean initialization
    inputBool = true;
    shouldFactorAgg = true;
    shouldSmooth = true;
    loadPressed = false;
    
    
    // Buffer Size determines # of FFT Bins
    // Ideally we want to make it as large as possible before it lags
    bufferSize = 4096;
    
    
    // Create FFT object
    fft = ofxFft::create(bufferSize, OF_FFT_WINDOW_BARTLETT);
    
    
    // Data array initialization
    // Builds list of frequencies from A2-G#7
    for(int i=-2; i<=3; i++){
        for(int j=0; j<chromaticScale.size(); j++){
            freqlist.push_back(chromaticScale[j]*pow(2, i));
        }
    }
    
    
    // Builds corresponding list of FFT bins
    fullBinList = freq2bin();
    
    
    // Resize data structures
    // In order to simplify passing octave + individual note data between
    // audio and drawing threads, we use one array where:
    //     audioData[0,oct_size-1] = octave data
    //     audioData[oct_size, wholeDataSize] = individual note data
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
    
    
    // GUI Initialization
    //--------------------------------------------------------------
    
    // Button Listeners
    inputToggle.addListener(this, &ofApp::inputPressed);
    outputToggle.addListener(this, &ofApp::outputPressed);
    factorToggle.addListener(this, &ofApp::factorAggPressed);
    smoothToggle.addListener(this, &ofApp::smoothPressed);
    loadButton.addListener(this, &ofApp::loadFile);
    playButton.addListener(this, &ofApp::playFile);
    
    // Main panel
    panel = gui.addGroup("Chromatic Profiler");
    panel->setPosition(10,10);
    panel->setDraggable(false);
    
    // Input mode
    audioModes = panel->addGroup("Audio Mode");
    audioModes->setShowHeader(false);
    audioModes->add(inputToggle.set("Stream Audio", true));
    audioModes->add(outputToggle.set("Play File", false));
    panel->addSpacer(0,5);
    
    // File Manager
    fileManager = panel->addGroup("File Manager");
    fileManager->setShowHeader(false);
    fileManager->add(filePath.set("Path/To/WavFile"));
    fileManager->add(loadButton.set("Load File"), ofJson({{"type", "fullsize"}, {"text-align", "center"}}));
    fileManager->add(playButton.set("Play / Pause"), ofJson({{"type", "fullsize"}, {"text-align", "center"}}));
    fileManager->minimize();
    panel->addSpacer(0,5);
    
    // Graph / Data Controls
    graphControls = panel->addGroup("Graph Controls");
    graphControls->add(smoothToggle.set("Smooth", true));
    graphControls->add(factorToggle.set("Factor Octaves", true));
    graphControls->minimize();
    
    // Resize graph and GUI layout
    updateLayout(WIN_WIDTH, WIN_HEIGHT);
}

//--------------------------------------------------------------
void ofApp::loadFile(){
    
    // Listener fires twice when pressed (click & release I think)
    // So this check makes sure it only prompts once
    if(!loadPressed){
        loadPressed = true;
        
        // Opens system dialog asking for a file
        // Checks to make sure it's .wav then loads
        ofFileDialogResult result = ofSystemLoadDialog("Load file");
        if(result.bSuccess) {
            string path = result.getPath();
            string name = result.getName();
            cout << path << endl;

            if(0 == path.compare (path.length() - 3, 3, "wav")){
                file.openFile(ofToDataPath(path,true));
                filePath.set(name);
            }
            else{
                ofSystemAlertDialog("Error: Must load .wav file");
                filePath.set("Invalid File Type");
            }
        }
    }
    
    loadPressed = false;
    
}

//--------------------------------------------------------------
void ofApp::playFile(){
    
    // Listener fires twice when pressed (click & release I think)
    // So this check makes sure it only prompts once
    if(!playPressed){
        playPressed = true;
        
        shouldPlayAudio = !shouldPlayAudio;
    }
    
    playPressed = false;
    
}

//--------------------------------------------------------------
void ofApp::inputPressed(bool &inputToggle){
    
    // If input button is set to true and mode is not set to input
    //     Set mode to input
    //     Set output button to false
    //     Hide file manager
    //     Reset graphs
    if(!inputBool && inputToggle){
        inputBool = true;
        outputToggle.set(false);
        fileManager->minimize();
        clearGraphs();
    }

    
}

//--------------------------------------------------------------
void ofApp::outputPressed(bool &outputToggle){
    
    // If output button is set to true and mode is set to input
    //     Set mode to output
    //     Set input button to false
    //     Show file manager
    //     Reset graphs
    if(inputBool && outputToggle){
        inputBool = false;
        inputToggle.set(false);
        fileManager->maximize();
        clearGraphs();
    }
    
}

//--------------------------------------------------------------
void ofApp::factorAggPressed(bool &factorToggle){
    shouldFactorAgg = factorToggle;
}

//--------------------------------------------------------------
void ofApp::smoothPressed(bool &smoothToggle){
    shouldSmooth = smoothToggle;
    
    // If smoothing is off, octave factoring must also be off
    if(!smoothToggle){
        factorToggle.set(false);
    }
}

//--------------------------------------------------------------
void ofApp::audioIn(ofSoundBuffer& buffer) {
    if(inputBool){
        
        // Grab input buffer, size, and number of channels
        auto& input = buffer.getBuffer();
        auto bufferSize = buffer.getNumFrames();
        auto numChannels = buffer.getNumChannels();
        
        // Store those in an StkFrames structure
        stk::StkFrames frames(bufferSize,numChannels);
        
        // Number of channels ~should~ always be 1, but if not, only use the left
        if(numChannels > 1){
            stk::StkFrames leftChannel(bufferSize,1);
            frames.getChannel(0, leftChannel, 0);
            
            // Audio buffer is interleaved {left frame, right frame, left frame, ...}
            // So copy left frames into right frame spots
            for (int i = 0; i < bufferSize ; i++) {
                input[2*i] = leftChannel(i,0);
                input[2*i+1] = leftChannel(i,0);
            }
        }
        
        // Send buffer to analysis
        analyzeAudio(input, bufferSize);
        
    }
}

//--------------------------------------------------------------
void ofApp::audioOut(ofSoundBuffer& buffer){
    
    if( !inputBool ){
        // Grab output buffer and size
        auto& output = buffer.getBuffer();
        auto bufferSize = buffer.getNumFrames();
        
        // If file is playing...
        if (shouldPlayAudio) {
            // Create StkFrames object with current frames
            stk::StkFrames frames(bufferSize,2);
            
            // Move file forward
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

        // Send to analysis
        analyzeAudio(output, (int)bufferSize);
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
    fft->setSignal(normalizedOut);
    
    // Audio listeners run in a separate thread
    // so we must ensure control before making changes to a shared variable
    const std::lock_guard<std::mutex> lock(mtx);
    
    // Max values for normalization
    float single_max = 0;
    float scale_max = 0;

    
    // Clear out summed data
    for(int i=0; i<oct_size; i++){
        audioData[i] = 0;
    }
    
    
    // Record new amplitudes for individual notes and summed notes
    int note;
    for(int i=0; i<scale_size; i++){
        // Simplification for summing notes across octaves (i.e. every A, B, C, etc.)
        note = i%12;
        
        float val = fft->getAmplitudeAtBin(fullBinList[i]);
        
        // record single note data / max
        audioData[i+oct_size] = val; // individual notes start at audioData[oct_size]
        if(val > single_max) {
            single_max = val;
        }
        
        
        // Sum each note across octaves
        audioData[note] += val;
        if(i >= scale_size-oct_size){ // Only need to update max sums on last octave
            if(audioData[note] > scale_max) scale_max = audioData[note];
        }
    }
    
    // Normalize summed data
    for(int i=0; i<oct_size; i++){
        audioData[i] /= scale_max;
    }

    
    // Normalize individual data
    for(int i=oct_size; i<wholeDataSize; i++){
        audioData[i] /= single_max;
    }
    
    // Note: This will output NaN when there is no sound, but this is accounted for
}

//--------------------------------------------------------------
void ofApp::clearGraphs(){
    
    // Reset graph during mode changes
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
    
    // copy analysis data to buffer for drawing
    memcpy(buffer, audioData, wholeDataSize*sizeof(float));
    
}

//--------------------------------------------------------------
void ofApp::exit(){
    // I feel like I should be using this but whenever I add stuff it breaks
}

//--------------------------------------------------------------
void ofApp::draw(){
 
    // Smoothing+Moving data from buffer
    
    // NaN checker (doesn't get used often but sometimes it's helpful
    if(buffer[0] != buffer[0]){
        // NaN. skip frame
        
        // Clear graphs if NaN and on output mode (usually means file not playing)
        if(!inputBool) clearGraphs();
        // Sometimes buffer will be NaN during input lag
        // So if we reset the graph each time (on input mode) it would be jumpy
        // Haven't observed the same issue on output
    }
    
    // At the moment, smoothing consists of:
    //   - averaging new value with previous to make it less 'jumpy'
    //   - squaring values below 0.5 to reduce noise
    else if(shouldSmooth){
        for(int i=0; i<wholeDataSize; i++){
                buffer[i] = ofClamp(buffer[i], 0, 1); // clamp new data
                
                // When octave data is factored in:
                //    value = 0.25(new value) + 0.5(old value) + 0.25(overall note value)
                if(shouldFactorAgg && i >= oct_size) {
                    displayData[i] = (buffer[i] + 2*displayData[i] + displayData[i%oct_size])/4.0;
                }
                else{
                    displayData[i] = (buffer[i] + 2*displayData[i])/3.0;
                }
                // Square values below 0.5 to reduce noise
                if(displayData[i] < 0.3) displayData[i] *= displayData[i];
        }
    }
    else{
        
        // When smoothing is off,
        for(int i=0; i<wholeDataSize; i++){
            buffer[i] = ofClamp(buffer[i], 0, 1); // clamp new data
            displayData[i] = buffer[i];           // write to display
        }
    }
    
    
    // Draw graphs
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
    
    // Draw border
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
    //    Data = Summed Octave
    //    Bars take up 80% of total width
    //    Margins take up rest of total width
    //    Max bar height is 95% of height
    //    Labels are displayed if bar width > 12 pixels
    int dataSize = oct_size;
    barWidth = (((float)width) * 0.8) / (float)dataSize;
    margin = (((float)width) - barWidth*dataSize) / ((float)dataSize+1);
    maxHeight = ((float)height)*0.95;
    y_offset = (float)(height + maxHeight)/2;
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
            yPosLabel = -6;
        }
        else{
            rect.height = -displayData[i]*maxHeight;
            yPosLabel = std::max((int)rect.height-6, (int)-maxHeight-10);
        }
        
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

// TODO: Merge these methods into one
//--------------------------------------------------------------
void ofApp::drawMultiOctave(float width, float height){
    
    // Draw border
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
    //    Data = Summed Octave
    //    Bars take up 80% of total width
    //    Margins take up rest of total width
    //    Max bar height is 95% of height
    //    No labels
    int dataSize = scale_size;
    barWidth = (((float)width) * 0.8) / (float)dataSize;
    margin = (((float)width) - barWidth*dataSize) / ((float)dataSize+1);
    maxHeight = ((float)height)*0.95;
    y_offset = (float)(height + maxHeight)/2;
    
    int x = 0;
    int noteNum = 0;
    int octaveNum = 2;
    int labelXOffset = max((barWidth-15)/2, 0);
    
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
        // y-axis is 'flipped' i.e. negative is upwards
        if(displayData[i] < 0.05 || displayData[i] != displayData[i]) {
            rect.height =  -3;
        }
        else{
            rect.height = -displayData[i]*maxHeight;
        }
        
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
    
    // Calculates new positions for graphs / controls when window is resized
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

