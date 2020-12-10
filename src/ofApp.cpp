#include "ofApp.h"
#include <string>
#include <math.h>
#include <algorithm>    // std::max

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetFrameRate(60);
    ofBackground(40);
    
    // Boolean initialization
    inputBool = true;
    shouldFactorAgg = true;
    shouldSmooth = true;
    loadPressed = false;
    viewMode = 0;
    
    
    // Buffer Size determines # of FFT Bins
    // Ideally we want to make it as large as possible before it lags
    bufferSize = 2048;
    
    
    // Create FFT object
    fft = ofxFft::create(bufferSize, OF_FFT_WINDOW_BARTLETT);
    
    
    
    
    // Data array initialization
    // Builds list of frequencies from A2-G#7
    for(int i=-2; i<=3; i++){
        for(int j=0; j<chromaticScale.size(); j++){
            freqlist.push_back(chromaticScale[j]*pow(2, i));
        }
    }
    
    overtoneBinList = new std::vector<int>[freqlist.size()];
    for(int i=0; i<freqlist.size(); i++){
        std::vector<int> binBucket;
        overtoneBinList[i] = binBucket;
    }
    
    // Builds corresponding list of FFT bins
    // Translate frequency list to bin list
    for(int i=0; i<freqlist.size(); i++){
        float bin = fft->getBinFromFrequency(freqlist[i], 44100);
        fullBinList.push_back((int)bin);
        
        // Save bin containing first five overtones of each note freq.
        for(int j=2; j<7; j++){
            if(freqlist[i]*j < 10000){
                float otBin = fft->getBinFromFrequency(freqlist[i]*j, 44100);
                overtoneBinList[i].push_back((int)otBin);
            }
        }
    }
    
    // Resize data structures
    // In order to simplify passing octave + individual note data between
    // audio and drawing threads, we use one array where:
    //     audioData[0,oct_size-1] = octave data
    //     audioData[oct_size, wholeDataSize] = individual note data
    oct_size = chromaticScale.size();
    scale_size = fullBinList.size();
    wholeDataSize = oct_size+scale_size;
    
    overtoneData = new float[scale_size];
    overtoneBuff = new float[scale_size];
    
    for(int i=0; i<oct_size; i++){
        keyDetector.push_back(notepair(0, i));
    }
    
    audioData = new float[wholeDataSize];
    buffer = new float[wholeDataSize];
    displayData = new float[wholeDataSize];
    
    
    // Initialization for display values
    for(int i=0; i<wholeDataSize; i++){
        displayData[i] = 0.01;
        buffer[i] = 0.01;
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
    viewModeToggle.addListener(this, &ofApp::viewModeChanged);
    closeButton.addListener(this, &ofApp::closeMenu);
    
    // Main panel
    gui.setupFlexBoxLayout();
    
    all = gui.addGroup("", ofJson({
        {"flex-direction", "row"},
        {"flex", 1},
        {"margin", 2},
        {"padding", 2},
        {"height", 30},
        {"background-color", "transparent"},
        {"width", "100%"},
        {"flex-wrap", "wrap"},
        {"show-header", false},
        {"position", "static"},
    }));
    
    
    
    // View mode
    viewControls = all->addGroup("View", ofJson({
        {"type", "panel"},
        {"align-self", "flex-start"}
        
    }));
    viewControls->loadTheme("default-theme.json");
    viewControls->setShowHeader(true);
    viewControls->setWidth(100);
    //viewControls->setPosition(0, 0);
    viewControls->add(viewModeLabel.set("Linear"));
    viewControls->add(viewModeToggle.set("Change View"), ofJson({{"type", "fullsize"}, {"text-align", "center"}}));
    viewControls->minimize();
    
    
    // Input mode
    audioModes = all->addGroup("Input", ofJson({
        {"type", "panel"},
        {"align-self", "flex-start"}
        
    }));
    audioModes->loadTheme("default-theme.json");
    audioModes->setShowHeader(true);
    audioModes->setWidth(100);
    //audioModes->setPosition(0, 0);
    
    // File Manager
    fileManager = audioModes->addGroup("File Manager");
    fileManager->loadTheme("default-theme.json");
    fileManager->setShowHeader(false);
    fileManager->add(filePath.set("Path/To/WavFile"));
    fileManager->add(loadButton.set("Choose .wav"), ofJson({{"type", "fullsize"}, {"text-align", "center"}}));
    fileManager->add(playButton.set("Play"), ofJson({{"type", "fullsize"}, {"text-align", "center"}}));
    fileManager->minimize();
    
    audioModes->add(inputToggle.set("Listen", true));
    audioModes->add(outputToggle.set("Play File", false));
    audioModes->minimize();
    

    
    // Graph / Data Controls
    graphControls = all->addGroup("Data", ofJson({
        {"type", "panel"},
        {"align-self", "flex-start"}
        
    }));
    //graphControls->setPosition(0, 0);
    graphControls->loadTheme("default-theme.json");
    graphControls->setWidth(100);
    graphControls->add(smoothToggle.set("Smooth", true));
    graphControls->add(factorToggle.set("Factor Octaves", true));
    graphControls->minimize();
    
    all->add(closeButton.set("X"), ofJson({
        {"type", "fullsize"},
        {"text-align", "center"},
        {"align-self", "flex-start"},
        {"margin", 5},
        {"width", 25},
        {"border-radius", 2}
    }));
    
    // Resize graph and GUI layout
    updateLayout(WIN_WIDTH, WIN_HEIGHT);
}

//--------------------------------------------------------------
void ofApp::closeMenu(){
    all->minimizeAll();
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
                try{
                    file.openFile(ofToDataPath(path,true));
                }
                catch(...){
                    cout << "oops" << endl;
                }
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
        if(shouldPlayAudio) playButton.setName("Play");
        else playButton.setName("Pause");
        
        
        shouldPlayAudio = !shouldPlayAudio;
    }
    
    playPressed = false;
    
}

//--------------------------------------------------------------
void ofApp::viewModeChanged(){
    // viewMode = 0 : linear charts
    // viewMode = 1 : polar chart
    
    if(viewMode == 0){
        viewMode = 1;
        viewModeLabel.set("Polar");
    }
    else{
        viewMode = 0;
        viewModeLabel.set("Linear");
    }
    
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
    else{
        outputToggle.set(true);
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
    else{
        inputToggle.set(true);
    }
    
}

//--------------------------------------------------------------
void ofApp::factorAggPressed(bool &factorToggle){
    shouldFactorAgg = factorToggle;
}

//--------------------------------------------------------------
void ofApp::smoothPressed(bool &smoothToggle){
    shouldSmooth = smoothToggle;
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
    float overtone_max = 0;
    
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
        
        // Sum overtones
        overtoneData[i] = 0;
        for(int j=0; j<overtoneBinList[i].size(); j++){
            overtoneData[i] += fft->getAmplitudeAtBin(overtoneBinList[i][j]);
        }
        if(overtoneData[i] > overtone_max) overtone_max = overtoneData[i];
    }
    
    // Normalize summed data
    for(int i=0; i<oct_size; i++){
        audioData[i] /= scale_max;
    }
    
    float sum = 0;
    p_note = 0;
    for(int i=oct_size; i<wholeDataSize; i++){
        audioData[i] /= single_max;
        sum += audioData[i];
        p_note += audioData[i]*(i);
    }

    if(sum != 0) p_note /= sum;

    
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
    memcpy(overtoneBuff, overtoneData, scale_size*sizeof(float));
    
    predictedNote = p_note;
    cout << predictedNote << endl;    //cout << keycode << endl;
    
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
                if(i >= oct_size) {
                    if(shouldFactorAgg){
                        float overtone = 0;
                        int count = 0;
                        for(int j=i+12; j<wholeDataSize; j+=12){
                            overtone += buffer[j];
                            count += 1;
                        }
                        overtone /= count;
                        displayData[i] = (buffer[i] + displayData[i] + overtone)/3.0;
                    }
                    else{
                        displayData[i] = (buffer[i] + 2*displayData[i] + displayData[i%12])/4.0;
                    }
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
    
    if(viewMode == 0){
        ofPushMatrix();
        ofTranslate(singleXOffset,singleYOffset);
        drawSingleOctave(singleW, singleH);
        ofPopMatrix();
        
        ofPushMatrix();
        ofTranslate(multiXOffset, multiYOffset);
        drawMultiOctave(multiW, multiH);
        ofPopMatrix();
    }
    if(viewMode == 1){
        ofPushMatrix();
        ofTranslate(0, 30);
        drawCircles(width, height-30);
        ofPopMatrix();
    }
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
    maxHeight = (-20)+((float)height)*0.95;
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
        
        ofRectangle rect;
        rect.x = x;
        rect.y = 0;
        rect.width = barWidth;

        // If rectangle height is below min. threshold, draw min rectangle
        // Note label follows rectangle if possible, otherwise sits on top of min rect.
        // y-axis is 'flipped' i.e. negative is upwards
        if(displayData[i] < 0.05 || displayData[i] > 1.0 || displayData[i] != displayData[i]) {
            rect.height =  -3;
            yPosLabel = -6;
        }
        else{
            rect.height = -displayData[i]*maxHeight;
            yPosLabel = std::max((int)rect.height-6, (int)-maxHeight-10);
        }
        
        
        float hue = (i%12)*(255.0/12);
        float sat = 100+displayData[i%12]*155;
        float brightness = 55+displayData[i]*200;
        
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
    int labelXOffset = ((barWidth-15)/2, 0);
    
    ofPushMatrix();
    ofTranslate(margin, y_offset); //Move to bottom-left corner for start
    
    //loop through raw values
    for(int i=oct_size; i<wholeDataSize; i++){
        
        ofPushStyle();
        
        
        ofRectangle rect;
        rect.x = x;
        rect.y = 0;
        rect.width = barWidth;

        // If rectangle height is below min. threshold, draw min rectangle
        // y-axis is 'flipped' i.e. negative is upwards
        if(displayData[i] < 0.05 || displayData[i] > 1.0 || displayData[i] != displayData[i]) {
            rect.height =  -3;
        }
        else{
            rect.height = -displayData[i]*maxHeight;
        }
        
        float hue = (i%12)*(255.0/12);
        float sat = 100+displayData[i]*155;
        float brightness = 55+displayData[i]*200;
        
        if(roundf(predictedNote) == i){
            brightness = 255;
            sat = 255;
        }
        
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

void ofApp::drawCircles(float width, float height){
    float constraint = min(width, height);
    float rMin = (constraint*0.1)/2;
    float rMax = (constraint*0.9)/2;
    
    float numOctaves = (wholeDataSize-oct_size)/oct_size;
    float r_inc = rMax / numOctaves;
    // first octave: rmin to r_inc*1
    //

    ofPushMatrix();
    ofTranslate(width/2, height/2);
    
    float deg = 0;
    float deg2;
    float rSmall = 0;
    float x1,y1,x2,y2,x3,y3,x4,y4;
    float rData, rad1, rad2, hue, sat, brightness;
    for(int i=oct_size; i<wholeDataSize; i++){
        if(i%12 == 0){
            rSmall += r_inc;
            deg = 0;
        }
        
        deg += 360/12;
        deg2 = deg+(360/12);
                
        if(displayData[i] < 0.1 || displayData[i] > 1 || displayData[i] != displayData[i]){
            rData = 0.1;
        }
        else{
            rData = displayData[i]*r_inc;
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
        sat = displayData[i]*255;
        brightness = displayData[i]*255;
        
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
    width = w;
    height = (h-45);
    h -= 35;
    // Calculates new positions for graphs / controls when window is resized
    int topPadding = h*0.02;
    int lrPadding = w*0.02;
    
    
    /*audioModes->setPosition(viewControls->getWidth(), 0);
    graphControls->setPosition(audioModes->getWidth()+viewControls->getWidth(), 0);
    
    // Controls are being cut off
    if(viewControls->getWidth()+audioModes->getWidth()+graphControls->getWidth() > width){
        // If two buttons fit...
        if(viewControls->getWidth()+audioModes->getWidth() < width){
            graphControls->setPosition(0, viewControls->getHeight()+5);
        }
        else{
            audioModes->setPosition(0, viewControls->getHeight()+5);
            graphControls->setPosition(0, viewControls->getHeight()+10+audioModes->getHeight());
        }
        
    }*/
    
    singleW = ((float)w*0.45);
    singleH =  ((float)h*0.45);
    multiW = ((float)w*0.96);
    multiH =  ((float)h*0.46);
    singleXOffset = w - (2*lrPadding+singleW);
    singleYOffset = 45+topPadding;
    multiXOffset = lrPadding;
    multiYOffset = 45+(singleH+2*topPadding);
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}



