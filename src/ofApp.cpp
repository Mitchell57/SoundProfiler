#include "ofApp.h"
#include <string>
#include <math.h>
#include <algorithm>    // std::max

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetFrameRate(60);
    ofBackground(20);
    
    // Boolean initialization
    inputBool = true;
    shouldFactorAgg = false;
    shouldSmooth = true;
    loadPressed = false;
    viewMode = 0;
    
    
    // Buffer Size determines # of FFT Bins
    // Ideally we want to make it as large as possible before it lags
    bufferSize = 2048;
    
    

    analysis.init(bufferSize);
    dm.init(WIN_WIDTH, WIN_HEIGHT);
    
    
    
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
    factorToggle.addListener(this, &ofApp::factorAggPressed);
    smoothToggle.addListener(this, &ofApp::smoothPressed);
    loadButton.addListener(this, &ofApp::loadFile);
    playButton.addListener(this, &ofApp::playFile);

    closeButton.addListener(this, &ofApp::closeMenu);
    
    // Main panel
    gui.setupFlexBoxLayout();
    
    all = gui.addGroup("Sonic Profiler", ofJson({
        {"flex-direction", "column"},
        {"flex", 1},
        {"margin", 2},
        {"padding", 2},
        {"background-color", "#181818"},
        {"flex-wrap", "wrap"},
        {"show-header", false},
        {"position", "static"},
    }));
    all->loadTheme("default-theme.json");
    
    
    

    // Display Mode
    displayParameters.setName("Display Mode");
    displayParameters.add(disp0.set("Linear",false));
    displayParameters.add(disp1.set("Polar",false));
    displayParameters.add(disp2.set("Raw",false));
    displayParameters.add(disp3.set("Oscillator",false));

        
    displayToggles = all->addGroup(displayParameters);
    displayToggles->setExclusiveToggles(true);
    displayToggles->loadTheme("default-theme.json");
    displayToggles->setConfig(ofJson({{"type", "radio"}}));
    
    
    
    // Input Mode (& file manager)
    inputParameters.setName("Input Source");
    inputParameters.add(input0.set("Microphone", false));
    inputParameters.add(input1.set("Play File", false));
    
    inputToggles = all->addGroup(inputParameters);
    inputToggles->setExclusiveToggles(true);
    inputToggles->loadTheme("default-theme.json");
    inputToggles->setConfig(ofJson({{"type", "radio"}}));
    
    fileManager = all->addGroup("File Manager");
    fileManager->loadTheme("default-theme.json");
    fileManager->setShowHeader(false);
    fileManager->add(filePath.set("Path/To/WavFile"));
    fileManager->add(loadButton.set("Choose File"), ofJson({{"type", "fullsize"}, {"text-align", "center"}}));
    fileManager->add(playButton.set("Play"), ofJson({{"type", "fullsize"}, {"text-align", "center"}}));
    fileManager->minimize();
    
    
    rawControls = all->addGroup("Raw Controls");
    rawControls->loadTheme("default-theme.json");
    rawControls->setShowHeader(false);
    
    linLogParameters.setName("FFT X-Axis");
    linLogParameters.add(lin.set("Linear", false));
    linLogParameters.add(log.set("Logarithmic", false));
    
    linLogToggles = rawControls->addGroup(linLogParameters);
    linLogToggles->setExclusiveToggles(true);
    linLogToggles->loadTheme("default-theme.json");
    linLogToggles->setConfig(ofJson({{"type", "radio"}}));
    
    rawControls->minimize();
    
    // Graph / Data Controls
    linearControls = all->addGroup("Mode Controls");
    linearControls->setShowHeader(false);
    linearControls->loadTheme("default-theme.json");
    //linearControls->add(smoothToggle.set("Smooth", true));
    linearControls->add(factorToggle.set("Factor Octaves", false));
    linearControls->minimize();
    
    // Minimize button
    all->add(closeButton.set("Minimize All"), ofJson({
        {"type", "fullsize"},
        {"text-align", "center"},
        {"align-self", "flex-start"},
        {"margin", 5},
        {"width", "100%"},
        {"border-radius", 2}
    }));
    
    displayToggles->getActiveToggleIndex().addListener(this, &ofApp::setDisplayMode);
    displayToggles->setActiveToggle(3);
    
    inputToggles->getActiveToggleIndex().addListener(this, &ofApp::setInputMode);
    inputToggles->setActiveToggle(0);
    
    linLogToggles->getActiveToggleIndex().addListener(this, &ofApp::setRawLinLog);
    linLogToggles->setActiveToggle(0);

    
    windowResized(WIN_WIDTH, WIN_HEIGHT);

}


void ofApp::setDisplayMode(int& index){
    switch(index){
            default: case 0:
                dm.setMode(DisplayMode::LINEAR);
                linearControls->maximize();
                rawControls->minimize();
                break;
            case 1:
                dm.setMode(DisplayMode::POLAR);
                linearControls->minimize();
                rawControls->minimize();
                break;
            case 2:
                dm.setMode(DisplayMode::RAW);
                linearControls->minimize();
                rawControls->maximize();
                break;
            case 3:
                dm.setMode(DisplayMode::OSC);
                linearControls->minimize();
                rawControls->minimize();
                break;

        }
}

void ofApp::setInputMode(int& index){
    switch (index) {
        case 0:
            inputBool = true;
            fileManager->minimize();
            break;
        case 1:
            inputBool = false;
            fileManager->maximize();
            break;
            
        default:
            break;
    }
}

void ofApp::setRawLinLog(int& index){
    switch (index) {
        case 0:
            dm.fftLinear = true;
            break;
        case 1:
            dm.fftLinear = false;
            break;
            
        default:
            break;
    }
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
void ofApp::factorAggPressed(bool &factorToggle){
    analysis.setAddOvertone(factorToggle);
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
        analysis.analyzeFrame(input, bufferSize);
        
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
        analysis.analyzeFrame(output, (int)bufferSize);
    }
}


//--------------------------------------------------------------
void ofApp::clearGraphs(){

   

}

//--------------------------------------------------------------
void ofApp::update(){
    // audio listeners run in a separate thread
    //   so we must ensure control before making changes to
    //   a shared variable
    

}

//--------------------------------------------------------------
void ofApp::exit(){
    // I feel like I should be using this but whenever I add stuff it breaks
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofPushMatrix();
    ofTranslate(controlWidth, 0);
    
    dm.draw(analysis);
    
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
    controlWidth = all->getWidth();
    dm.updateLayout(w - controlWidth, h);
}



//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}



