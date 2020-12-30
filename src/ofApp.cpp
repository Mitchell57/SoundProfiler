#include "ofApp.h"
#include <string>
#include <math.h>
#include <algorithm>    // std::max

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetFrameRate(60);
    ofBackground(20);
//    ofSetWindowShape(getPixelScreenCoordScale()*1024, win->getPixelScreenCoordScale()*768);
    
    // Buffer Size determines # of FFT Bins
    // Ideally we want to make it as large as possible before it lags
    bufferSize = 2048;
    
    // Initialize analysis + display classes
    analysis.init(bufferSize);
    //dm.init(WIN_WIDTH, WIN_HEIGHT);
    
    
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
    
    //-------------------------------------------------------------------------------------
    // GUI Initialization
    //-------------------------------------------------------------------------------------
    
    // main panel
    //-------------------------------------------------------------------------------------
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
    
    
    // input mode / file manager
    //-------------------------------------------------------------------------------------
    inputParameters.setName("Input Source");
    inputParameters.add(input0.set("Microphone", false));
    inputParameters.add(input1.set("Play File", false));
    
    inputToggles = all->addGroup(inputParameters);
    inputToggles->setExclusiveToggles(true);
    inputToggles->loadTheme("default-theme.json");
    inputToggles->setConfig(ofJson({{"type", "radio"}}));
    
    fileManager = inputToggles->addGroup("File Manager");
    fileManager->loadTheme("default-theme.json");
    fileManager->setShowHeader(false);
    fileManager->add(filePath.set("Path/To/WavFile"));
    fileManager->add(loadButton.set("Choose File"), ofJson({{"type", "fullsize"}, {"text-align", "center"}}));
    
    playbackControls = fileManager->addGroup("Playback",ofJson({
        {"flex-direction", "row"},
        {"flex", 0},
        {"justify-content", "center"},
        {"margin", 2},
        {"padding", 2},
        {"flex-wrap", "wrap"},
        {"show-header", false},
    }));
    playbackControls->loadTheme("default-theme.json");
    playbackControls->add(playButton.set("Play"), ofJson({{"type", "fullsize"}, {"text-align", "center"}, {"width", "45%"}}));
    playbackControls->add(resetButton.set("Reset"), ofJson({{"type", "fullsize"}, {"text-align", "center"}, {"width", "45%"}}));
    playbackControls->minimize();
    fileManager->minimize();

//
//
//    // osc controls
//    //-------------------------------------------------------------------------------------
//    oscControls = modeControls->addGroup("Mode Controls");
//    oscControls->setShowHeader(false);
//    oscControls->loadTheme("default-theme.json");
//
//    oscControls->add(colorWidth.set("Color Width", 120, 0, 255));
//    oscControls->add(colorShift.set("Color Shift", 0, 0, 255));
//    oscControls->add(smooth.set("Smooth", 0.25, 0., 1.));
//    oscControls->add(factorToggle.set("Factor Octaves", false));
    
    
    // misc
    //-------------------------------------------------------------------------------------
    dc.setup(&analysis, ofGetWidth(), ofGetHeight(), all);
    
    all->add(minimizeButton.set("Collapse All"), ofJson({{"type", "fullsize"}, {"text-align", "center"}}));
    
    // listeners
    //-------------------------------------------------------------------------------------
    
    // input radio
    inputToggles->getActiveToggleIndex().addListener(this, &ofApp::setInputMode);
    inputToggles->setActiveToggle(0);
    
    // file buttons
    loadButton.addListener(this, &ofApp::loadFile);
    playButton.addListener(this, &ofApp::playFile);
    resetButton.addListener(this, &ofApp::restartFile);
    
    // minimize button
    minimizeButton.addListener(this, &ofApp::minimizePressed);
    

    // Call resize to update control panel width and adjust drawing boxes
    windowResized(WIN_WIDTH, WIN_HEIGHT);

}


//-------------------------------------------------------------------------------------
// listeners
//-------------------------------------------------------------------------------------


//--------------------------------------------------------------
// Update input source and show/hide file manager
void ofApp::setInputMode(int& index){
    switch (index) {
        case 0:
            inputBool = true;
            fileManager->minimize();
            break;
        case 1:
            inputBool = false;
            fileManager->maximize();
            if(fileLoaded) playbackControls->maximize();
            break;
            
        default:
            break;
    }
}


//--------------------------------------------------------------
// Open system dialog and allow user to choose .wav file
void ofApp::loadFile(){
    
    // Listener fires twice when pressed (click & release)
    // So this check makes sure it only prompts once
    if(!loadPressed){
        loadPressed = true;
        
        // Opens system dialog asking for a file
        // Checks to make sure it's .wav then loads
        
        ofFileDialogResult result = ofSystemLoadDialog("Load file");
        
        if(result.bSuccess) {
            filePath.set("path/to/file.wav");
            fileLoaded = false;
            playbackControls->minimize();
            string path = result.getPath();
            string name = result.getName();
            cout << path << endl;

            if(0 == path.compare (path.length() - 3, 3, "wav")){
                try{
                    file.openFile(ofToDataPath(path,true));
                    filePath.set(name);
                    fileLoaded = true;
                    playbackControls->maximize();
                }
                catch(...){
                    ofSystemAlertDialog("Invalid File: Must load .wav file");
                }
            }
            else
            {
                ofSystemAlertDialog("Invalid File: Must load .wav file");
            }
        }
    }
    
    loadPressed = false;
}

//--------------------------------------------------------------
// Toggle whether a file should be playing or not
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
// Toggle whether a file should be playing or not
void ofApp::restartFile(){
    
    // Listener fires twice when pressed (click & release I think)
    // So this check makes sure it only prompts once
    if(!resetPressed){
        resetPressed = true;
        file.reset();
    }
    
    resetPressed = false;
    
}


//--------------------------------------------------------------
// Collapse main panels
void ofApp::minimizePressed(){
    dc.minimize();
    inputToggles->minimize();
}

void ofApp::maximize(){
    dc.maximize();
    inputToggles->maximize();
}


//-------------------------------------------------------------------------------------
// audio
//-------------------------------------------------------------------------------------

//--------------------------------------------------------------
// Retrieves and formats current frame of audio input then sends to analysis
void ofApp::audioIn(ofSoundBuffer& buffer) {
    if(inputBool)
    {
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
// Retrieves and formats current frame of audio output then sends to analysis
void ofApp::audioOut(ofSoundBuffer& buffer){
    if(!inputBool )
    {
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


//-------------------------------------------------------------------------------------
// builtins
//-------------------------------------------------------------------------------------

//--------------------------------------------------------------
void ofApp::draw(){
//    ofScale(win->getPixelScreenCoordScale(), win->getPixelScreenCoordScale(), 1.0);
    
    ofPushMatrix();
    ofTranslate(controlWidth, 0);
    
    dc.draw();
    
    ofPopMatrix();
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    // TODO: Add keyboard shortcuts
    if(key == 'm'){
        dc.incMode();
    }
    if(key == 'q'){
        minimizePressed();
    }
    if(key == 'w'){
        maximize();
    }
    if(key == '1'){
        dc.setMode(0);
    }
    if(key == '2'){
        dc.setMode(1);
    }
    if(key == '3'){
        dc.setMode(2);
    }
}


//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    controlWidth = all->getWidth();
    dc.updateLayout(w - controlWidth, h);
}

//--------------------------------------------------------------
void ofApp::update(){
    dc.update();
}

//--------------------------------------------------------------
void ofApp::exit(){
    // I feel like I should be using this but whenever I add stuff it breaks
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
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}



