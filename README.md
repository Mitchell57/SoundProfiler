# SoundProfiler
Audio analysis based on chromatic value, using openFrameworks

Dependencies:
- ofxFft: Used for frequency analysis
- ofxStk: Used for audio input / output, audio file management
- ofxGuiExtended: Used to display data and control settings

## Installation  
1. Install [openFrameworks](https://openframeworks.cc/download/)
2. (optional) Install SoundFlower for input streaming
    - Download and install [SoundFlower](https://github.com/mattingalls/Soundflower/releases/tag/2.0b2)
    - Open Audio MIDI Setup
    - Create new Multi-Output Device using `+` at bottom left corner
    - Select computer output and Soundflower(2ch)
    - Right-click new device and select 'Use This Device for Sound Output'
    - Right-click Soundflower (2ch) and select 'Use This Device for Sound Input'
3. Clone addons
    - Navigate to openFrameworks directory
    - Enter addons folder `cd addons`
    - Clone ofxFft `git clone git@github.com:kylemcdonald/ofxFft.git`
    - Clone ofxStk `git clone git@github.com:Ahbee/ofxStk.git`
    - Clone ofxGuiExtended `git clone git@github.com:frauzufall/ofxGuiExtended.git`
4. Clone Project
    - Navigate to openFrameworks directory
    - Make new folder for lab project `mkdir apps/CS1L ; cd apps/CS1L`
    - Clone project `git clone git@github.com:Mitchell57/SoundProfiler.git'
5. Open project in XCode and run!
