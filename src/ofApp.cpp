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
        dm.setMode(DisplayMode::POLAR);
    }
    else{
        viewMode = 0;
        viewModeLabel.set("Linear");
        dm.setMode(DisplayMode::LINEAR);
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
    
    dm.draw(analysis);
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
    dm.updateLayout(w, h);
}



//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}



