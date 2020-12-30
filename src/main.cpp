#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
#ifdef TARGET_OPENGLES
    ofGLESWindowSettings settings;
    settings.glesVersion=2;
#else
    ofGLWindowSettings settings;
    settings.setGLVersion(3,2);
#endif
    settings.setSize(WIN_WIDTH, WIN_HEIGHT);
    shared_ptr<ofAppBaseWindow> win = ofCreateWindow(settings);
    
    // weirdly enough this fixes the retina downscaling issue
    win->toggleFullscreen();
    win->toggleFullscreen();

    // this kicks off the running of my app
    // can be OF_WINDOW or OF_FULLSCREEN
    // pass in width and height too:
    ofRunApp(new ofApp());

}
