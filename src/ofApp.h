#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#define MAX_N_PTS         1500


//Particle class
class Particle {
public:
    Particle();                //Class constructor
    void setup();              //Start particle
    void update( float dt );   //Recalculate physics
    void draw();               //Draw particle
    
    ofPoint pos1;
    ofPoint pos2;
    ofPoint pos3; //Position
    ofPoint vel;               //Velocity
    float time;                //Time of living
    float lifeTime;            //Allowed lifetime
    bool live;                 //Is particle live
};

//Control parameters class
class Params {
public:
    void setup();
    ofPoint eCenter1;    //Emitter center
    ofPoint eCenter2;    //Emitter center
    ofPoint eCenter3;    //Emitter center

    float eRad;         //Emitter radius
    float velRad;       //Initial velocity limit
    float lifeTime;     //Lifetime in seconds
    
    float rotate;   //Direction rotation speed in angles per second
    
    float force;       //Attraction/repulsion force inside emitter
    float spinning;    //Spinning force inside emitter
    float friction;    //Friction, in the range [0, 1]
    float transparency;
    
    float eRad1;         //Emitter radius
    float velRad1;       //Initial velocity limit
    float force1;       //Attraction/repulsion force inside emitter
    
    float eRad2;         //Emitter radius
    float velRad2;       //Initial velocity limit
    float force2;       //Attraction/repulsion force inside emitter

    
//    float SO2_wind;
//    float NOX_wind;
//    float CO2_wind;
//    float SO2_nuclear;
//    float NOX_nuclear;
//    float CO2_nuclear;
//    float SO2_coal;
//    float NOX_coal;
//    float CO2_coal;
//
//    float SO2_total;
//    float NOX_total;
//    float CO2_total;
//    float emission_total;
    
    ofxCvContourFinder contourFinder1;
    ofxCvContourFinder contourFinder2;
    ofxCvContourFinder contourFinder3;
    
    
};

extern Params params; //Declaration of a global variable


//------------ GUI --------------
//Slider class
class Slider {
public:
    string title;        //Title
    ofRectangle rect;    //Rectangle for drawing
    float *value;       //Pointer to value which the slider changes
    float minV, maxV;   //Minimum and maximum values
};

//Interface class, which manages sliders
class Interface {
public:
    void setup();
    void addSlider( string title, float *value, float minV, float maxV );
    void draw();
    
    void save( int index );        //Save preset
    void load( int index );        //Load preset
    
    void mousePressed( int x, int y );
    void mouseDragged( int x, int y );
    void mouseReleased (int x, int y );
    
    vector<Slider> slider;    //Array of sliders
    int selected;            //Index of selected slider
};

typedef struct {
    
    float     x;
    float     y;
    bool     bBeingDragged;
    bool     bOver;
    float     radius;
    
}draggableVertex;


class ofApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    void resetParticles();
    
    void keyPressed  (int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    ofVec3f    pts[MAX_N_PTS];
    int        nPts;
    int        r1;
    int        r2;
    int        r3;
    ofPoint eCenter;    //Emitter center
    ofPoint pos;
    ofPoint vel;
    ofPoint frc;
    
    vector<Particle> p;      //Particles
    ofFbo fbo;            //Off-screen buffer for trails
    
    float history;        //Control parameter for trails
    float time0;          //Time value for computing dt
    
    float bornRate;       //Particles born rate per second
    float bornCount;      //Integrated number of particles to born
    
    //GUI
    Interface interf;
    bool drawInterface;
    bool drawCamera;
    
    bool bDrawnAnything;
    
    ofVideoGrabber movie;
    ofxCvColorImage rgb,hsb;
    ofxCvGrayscaleImage hue,sat,bri,filtered,filter1,filter2,filter3;
    ofxCvContourFinder contourFinder1;
    ofxCvContourFinder contourFinder2;
    ofxCvContourFinder contourFinder3;
    
    ofImage background;
    ofImage citizen;
    ofImage citizenh;
    ofImage citizens;
    
    
    ofVideoGrabber vidGrabber;
    int camWidth;
    int camHeight;
    int w,h;
    int trackHue1;
    int trackHue2;
    int trackHue3;
    
    
    void drawNoisyArmRect (float w, float h);
    
    
    string currentModeStr;
    vector <ofPoint> attractPoints;
    vector <ofPoint> attractPointsWithMovement;
    
    float SO2_wind;
    float NOX_wind;
    float CO2_wind;
    float SO2_nuclear;
    float NOX_nuclear;
    float CO2_nuclear;
    float SO2_coal;
    float NOX_coal;
    float CO2_coal;
    
    float SO2_total;
    float NOX_total;
    float CO2_total;
    float emission_total;
    
};

