#include "ofApp.h"
Params param;        //Definition of global variable
//--------------------------------------------------------------


void ofApp::setup(){
    ofSetVerticalSync(true);
    
    ofSetCircleResolution(256);
    
    ofSetVerticalSync(true);
    ofBackground(255,255,255);
    ofSetFrameRate(60);
    
    //no change
    w = 640;
    h = 480;
    
    //webcam: select video input
    vector<ofVideoDevice> devices = movie.listDevices();
    for(int i = 0; i < devices.size(); i++){
        if(devices[i].bAvailable){
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName;
        }else{
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName << " - unavailable ";
        }
    }
    
    movie.setDeviceID(1);
    movie.setDesiredFrameRate(60);
    vidGrabber.initGrabber(w, h);
    movie.initGrabber(w, h);
    //reserve memory for cv images
    rgb.allocate(w, h);
    hsb.allocate(w, h);
    hue.allocate(w, h);
    sat.allocate(w, h);
    bri.allocate(w, h);
    filter1.allocate(w, h);
    filter2.allocate(w, h);
    filter3.allocate(w, h);
    
    background.load("images/background.png");
    citizen.load("images/citizen.png");
    citizenh.load("images/citizenh.png");
    citizens.load("images/citizens.png");
    
//---------------------------------------------
    
    //Allocate drawing buffer
    int w = ofGetWidth();
    int h = ofGetHeight();
    fbo.allocate( w, h, GL_RGB32F_ARB );
    
    //Fill buffer with white color
    fbo.begin();
    ofBackground(255, 255, 255);
    fbo.end();
    
    //Set up parameters
    param.setup();        //Global parameters
    history = 0.0;
    bornRate = 5;
    
    bornCount = 0;
    time0 = ofGetElapsedTimef();
    
    
}

//--------------------------------------------------------------



void ofApp::update(){
    
    
    movie.update();
    
    if (movie.isFrameNew()) {
        
        //copy webcam pixels to rgb image
        //        rgb.setFromPixels(movie.getPixels(), w, h);
        rgb.setFromPixels(movie.getPixels());
        //mirror horizontal
        rgb.mirror(false, true);
        
        //duplicate rgb
        hsb = rgb;
        //convert to hsb
        hsb.convertRgbToHsv();
        //        rgb.convertHsvToRgb();
        
        //store the three channels as grayscale images
        hsb.convertToGrayscalePlanarImages(hue, sat, bri);
        
        //        ofColor c = ofColor::fromHsb( 0, 255, 255 ); // bright red
        //        c.setHue( 128 ); // now bright cyan
        
        //filter image based on the hue value were looking for
        //Goal is to track 3 different colors and draw different circles accordingly
        
        for (int i=0; i<w*h; i++) {
            
            filter1.getPixels()[i] = ofInRange(hue.getPixels()[i],trackHue1-1,trackHue1+1) ? 360:0; // trackHue1 = pink wind
            filter2.getPixels()[i] = ofInRange(hue.getPixels()[i],trackHue2-1,trackHue2+1) ? 360:0; // trackHue2 = yellow coal
            filter3.getPixels()[i] = ofInRange(hue.getPixels()[i],trackHue3-1,trackHue3+1) ? 360:0; // trackHue3 = blue nuclear
        }
        filter1.flagImageChanged();
        filter2.flagImageChanged();
        filter3.flagImageChanged();
        
        //run the contour finder on the filtered image to find blobs with a certain hue
        contourFinder1.findContours(filter1, 50, w*h/4, 1, false, false); // pink
        contourFinder2.findContours(filter2, 50, w*h/4, 1, false, false); // yellow
        contourFinder3.findContours(filter3, 50, w*h/4, 1, false, false); // blue
        
        for (int i=0; i<contourFinder1.nBlobs; i++) {
            param.eCenter1 = ofPoint(  contourFinder1.blobs[i].centroid.x, contourFinder1.blobs[i].centroid.y);
        }
        for (int i=0; i<contourFinder2.nBlobs; i++) {
            param.eCenter2 = ofPoint(  contourFinder2.blobs[i].centroid.x, contourFinder2.blobs[i].centroid.y );
        }
        for (int i=0; i<contourFinder3.nBlobs; i++) {
            param.eCenter3 = ofPoint(  contourFinder3.blobs[i].centroid.x, contourFinder3.blobs[i].centroid.y );
        }
    }
    
    //Compute dt
    float time = ofGetElapsedTimef();
    float dt = ofClamp( time - time0, 0, 0.1 );
    time0 = time;
    
    //Delete inactive particles
    int i=0;
    while (i < p.size()) {
        if ( !p[i].live ) {
            p.erase( p.begin() + i );
        }
        else {
            i++;
        }
    }
    
    //Born new particles
    bornCount += dt * bornRate;      //Update bornCount value
    if ( bornCount >= 1 ) {          //It's time to born particle(s)
        int bornN = int( bornCount );//How many born
        bornCount -= bornN;          //Correct bornCount value
        for (int i=0; i<bornN; i++) {
            Particle newP;
            newP.setup();            //Start a new particle
            p.push_back( newP );     //Add this particle to array
        }
    }
    
    //Update the particles
    for (int i=0; i<p.size(); i++) {
        p[i].update( dt );
    }
    
}




void Params::setup() {
    
    
//    eCenter = ofPoint( ofGetWidth()/2, ofGetHeight()/2 );
    eRad = 370;
    velRad = 365;
    lifeTime = 7.6;
    rotate = 0;
    force = -40;
    spinning = 14;
    friction = 0.01;
    
    eRad1 = 370;
    velRad1 = 365;
    force1 = -40;
    
    eRad2 = 370;
    velRad2 = 365;
    force2 = -40;
    

    
}
Particle::Particle() {
    live = false;
}

ofPoint randomPointInCircle( float maxRad ){
    ofPoint pnt;
    float rad = ofRandom( 0, maxRad );
    float angle = ofRandom( 0, M_TWO_PI );
    pnt.x = cos( angle ) * rad;
    pnt.y = sin( angle ) * rad;
    return pnt;
}

void Particle::setup() {
    pos1 = param.eCenter1 + randomPointInCircle( param.eRad );
    pos2 = param.eCenter2 + randomPointInCircle( param.eRad );
    pos3 = param.eCenter3 + randomPointInCircle( param.eRad );
    vel = randomPointInCircle( param.velRad );
    time = 0;
    lifeTime = param.lifeTime;
    live = true;
}

//--------------------------------------------------------------
void Particle::update( float dt ){
    if ( live ) {
        //Rotate vel
        vel.rotate( 0, 0, param.rotate * dt );
        
        ofPoint acc;         //Acceleration
        ofPoint delta1 = pos1 - param.eCenter1;
        ofPoint delta2 = pos2 - param.eCenter2;
        ofPoint delta3 = pos3 - param.eCenter3;
        float len1 = delta1.length();
        float len2 = delta2.length();
        float len3 = delta3.length();
        if ( ofInRange( len1, 0, param.eRad ) ) {
            delta1.normalize();
            //Attraction/repulsion force
            acc += delta1 * param.force;
            //Spinning force
            acc.x += -delta1.y * param.spinning;
            acc.y += delta1.x * param.spinning;
        }
        if ( ofInRange( len2, 0, param.eRad ) ) {
            delta2.normalize();
            //Attraction/repulsion force
            acc += delta2 * param.force;
            //Spinning force
            acc.x += -delta2.y * param.spinning;
            acc.y += delta2.x * param.spinning;
        }
        if ( ofInRange( len3, 0, param.eRad ) ) {
            delta3.normalize();
            //Attraction/repulsion force
            acc += delta3 * param.force;
            //Spinning force
            acc.x += -delta3.y * param.spinning;
            acc.y += delta3.x * param.spinning;
        }
        
        vel += acc * dt;            //Euler method
        vel *= (1-param.friction);  //Friction
        
        //Update pos
        pos1 += vel * dt;
        pos2 += vel * dt;
        pos3 += vel * dt;
        
        //Update time and check if particle should die
        time += dt;
        if ( time >= lifeTime ) {
            live = false;   //Particle is now considered as died
        }
    }
}

//--------------------------------------------------------------
void Particle::draw(){
    
    if ( live ) {
        //Compute size
        float size = ofMap(
        fabs(time - lifeTime/2), 0, lifeTime/2, 3, 1 );
        
        //Compute color
        //        ofColor color = ofColor::black;
        float color = ofMap( time, 0, lifeTime, 100, 255 );
        ofSetColor( color );
        
        ofDrawCircle( pos1, size );  //Draw particle
        ofDrawCircle( pos2, size );  //Draw particle
        ofDrawCircle( pos3, size );  //Draw particle
    }
}

//--------------------------------------------------------------


void ofApp::draw(){
    

    
    
    //1. Drawing to buffer
    fbo.begin();

    
    //Draw semi-transparent white rectangle
    //to slightly clearing a buffer (depends on history value)
    
    ofEnableAlphaBlending();         //Enable transparency
    
    float alpha = (1-history) * 255;
    ofSetColor( 255, 255, 255, alpha );
    ofFill();
    ofDrawRectangle( 0, 0, ofGetWidth(), ofGetHeight() );
    
    ofDisableAlphaBlending();        //Disable transparency
    
    
    //floating particles
    
    ofSetColor(200);
    ofNoFill();
    
//
//    for (int i = 0; i < 13; i++){
//        ofDrawRectangle(150+(i*105),100, 100,100);
//        ofDrawRectangle(150+(i*105),205,100,100);
//        ofDrawRectangle(150+(i*105),310,100,100);
//        ofDrawRectangle(150+(i*105),415,100,100);
//        ofDrawRectangle(150+(i*105),520,100,100);
//        ofDrawRectangle(150+(i*105),625,100,100);
//        ofDrawRectangle(150+(i*105),730,100,100);
//
//    }

    
    ofSetColor(190);
    ofSetHexColor(0x000000);
    ofNoFill();
    ofBeginShape();
    for (int i = 0; i < nPts; i++){
        ofVertex(pts[i].x, pts[i].y);
    }
    ofSetColor(255,255,255);
    

    //draw all cv images
//    rgb.draw(0,0);
//        hsb.draw(640,0);
//        hue.draw(0,480);
//        sat.draw(640,480);
//        bri.draw(1280,480);
    //    filtered.draw(0,960);
   //      contourFinder.draw(0,0);
    
    if(drawCamera){
        rgb.draw(0,0);
    }else {}
    
    
    
    ofSetColor(0, 0, 230);
    ofFill();
    
    
    //Draw the particles
    ofFill();
    for (int i=0; i<p.size(); i++) {
        p[i].draw();
    }
    
    fbo.end();
    
    //2. Draw buffer on the screen
    ofSetColor( 255, 255, 255 );
    
    fbo.draw( 0, 0 );
    background.draw(0,0,1600,1048);
 
    
    SO2_wind =0.055;
    NOX_wind =0.065;
    CO2_wind =15.5;
    
    SO2_coal =1.4;
    NOX_coal =1.2;
    CO2_coal =915;
    
    SO2_nuclear =0.0205;
    NOX_nuclear =0.025;
    CO2_nuclear =66;
    
    SO2_total= contourFinder1.nBlobs*SO2_wind + contourFinder2.nBlobs*SO2_coal + contourFinder3.nBlobs*SO2_nuclear;
    NOX_total= contourFinder1.nBlobs*NOX_wind + contourFinder2.nBlobs*NOX_coal + contourFinder3.nBlobs*NOX_nuclear;
    CO2_total= contourFinder1.nBlobs*CO2_wind + contourFinder2.nBlobs*CO2_coal + contourFinder3.nBlobs*CO2_nuclear;
    emission_total = SO2_total+NOX_total+CO2_total;
    
    float x3 = 850;
    float y3 = 200;
    
    //draw blue circles for found blobs
    for (int i=0; i<contourFinder1.nBlobs; i++) {
        float x = contourFinder1.blobs[i].centroid.x+100*cos(ofGetElapsedTimef()/3.0f);
        float y = contourFinder1.blobs[i].centroid.y+100*sin(ofGetElapsedTimef()/2.5f);
        float x0 = contourFinder1.blobs[i].centroid.x+100*cos(ofGetElapsedTimef()/1.5f);
        float y0 = contourFinder1.blobs[i].centroid.y+100*sin(ofGetElapsedTimef()/2.5f);

//        ofSetColor(255, 153, 170);
//        ofDrawCircle(contourFinder1.blobs[i].centroid.x, contourFinder1.blobs[i].centroid.y, 30); // drawing pink circles on grid
        ofSetColor(183, 73, 91);
        ofDrawBitmapString("wind turbine", contourFinder1.blobs[i].centroid.x, contourFinder1.blobs[i].centroid.y+50);
        ofSetColor(255);
        ofEnableAlphaBlending();
        
        if( emission_total > 900){
        citizenh.draw(x0,y0,50,50);
        }else {
        citizenh.draw(x0,y0,50,50);
        }

      
    }
    
    for (int i=0; i<contourFinder2.nBlobs; i++) {
        float x1 = contourFinder2.blobs[i].centroid.x+50*cos(ofGetElapsedTimef()*1.0f);
        float y1 = contourFinder2.blobs[i].centroid.y+100*sin(ofGetElapsedTimef()/3.5f);
//        ofSetColor(244, 215, 100);
//        ofDrawCircle(contourFinder2.blobs[i].centroid.x, contourFinder2.blobs[i].centroid.y, 30); // drawing yellow circles on grid
        ofSetColor(191, 115, 22);
        ofDrawBitmapString("coal power plant", contourFinder2.blobs[i].centroid.x, contourFinder2.blobs[i].centroid.y+50);
        ofSetColor(255);
        ofEnableAlphaBlending();
        citizens.draw(x1, y1,50,50);
    }
    
        for (int i=0; i<contourFinder3.nBlobs; i++) {
            float x2 = contourFinder3.blobs[i].centroid.x+30*cos(ofGetElapsedTimef()*2.0f);
            float y2 = contourFinder3.blobs[i].centroid.y+100*sin(ofGetElapsedTimef()*0.75f);
//            ofSetColor(100, 143, 244);
//            ofDrawCircle(contourFinder3.blobs[i].centroid.x, contourFinder3.blobs[i].centroid.y, 30); // drawing blue circles on grid
            ofSetColor(9, 37, 104);
            ofDrawBitmapString("nuclear plant", contourFinder3.blobs[i].centroid.x+50, contourFinder3.blobs[i].centroid.y+50);
            ofSetColor(255);
            ofEnableAlphaBlending();
            citizenh.draw(x2, y2,50,50);
        }
    
//    ofSetColor(0, 0, 230);
//    //    ofSetHexColor(0x000000);
//    ofNoFill();
    ofBeginShape();
    ofEndShape();

    
    //print data
    ofSetColor(107, 114, 124);
    stringstream reportStream;
    reportStream << "Pink hue value " << trackHue1 << "    " << "pink blobs found " << contourFinder1.nBlobs   <<endl
    << "Yellow hue value " << trackHue2 << "  " << "yellow blobs found " << contourFinder2.nBlobs <<endl
    << "Blue hue value " << trackHue3 << "    " << "blue blobs found " << contourFinder3.nBlobs  <<endl;
    ofDrawBitmapString(reportStream.str(), 20, 950);
    
    //data monitor
    ofSetColor(51, 57, 66);
    stringstream townData;
    townData<< "Town size: " << (contourFinder1 .nBlobs*3) + (contourFinder2.nBlobs*4) + (contourFinder3.nBlobs*8)<< " people" << endl //citizens recruited based on the electricity supplied
    << "Town capita: $" << ((contourFinder1 .nBlobs*3) + (contourFinder2.nBlobs*4) + (contourFinder3.nBlobs*8))*101.8 <<endl  //every citizen generates $101.8/day
    << "Air Quality Index: " << (contourFinder1 .nBlobs*0.43) + (contourFinder2.nBlobs*0.8) + (contourFinder3.nBlobs*3.75) << " level" <<endl;
    ofDrawBitmapString(townData.str(), 20, 30);
    
    stringstream pollutantData;
    pollutantData<< "Air Pollutants and Greenhouse Gases" <<endl
    << "  " <<endl
//    << "Sulfur Dioxide:               " << (contourFinder1 .nBlobs * SO2_wind) + (contourFinder2.nBlobs* SO2_coal) + (contourFinder3.nBlobs* SO2_nuclear) << "grams/cubic meter" <<endl
//    << "Nitrogen Oxide:               " << (contourFinder1 .nBlobs* NOX_wind) + (contourFinder2.nBlobs* NOX_coal) + (contourFinder3.nBlobs* NOX_nuclear) << "grams/cubic meter" <<endl
//    << "Carbon Dioxide - Equivalent:  " << (contourFinder1 .nBlobs* CO2_wind) + (contourFinder2.nBlobs*CO2_coal) + (contourFinder3.nBlobs*CO2_nuclear) << "grams/cubic meter" <<endl
    << "Total SO2:  " <<SO2_total << "grams/cubic meter" <<endl
    << "Total NOX:  " << NOX_total << "grams/cubic meter" <<endl
    << "Total CO2:  " << CO2_total << "grams/cubic meter" <<endl
    << "Total Emission:  " << emission_total << "grams/cubic meter" <<endl;
    ofDrawBitmapString(pollutantData.str(), 850, 30);
    
    
  
//    for (int i=0; i<contourFinder1.nBlobs; i++) {
//    ofSetColor(255);
//    ofEnableAlphaBlending();
//    citizenh.draw(x3, y3,100,100);
//    citizens.draw(x3,y3,100,100);
//    ofDisableAlphaBlending();}

    //---------------------------------------------------------
    
}



void Interface::setup(){
    selected = -1;
}

void Interface::addSlider( string title, float *value, float minV, float maxV ){
    Slider s;
    s.title = title;
    s.rect = ofRectangle( 20, 60 + slider.size() * 40, 150, 30 );
    s.value = value;
    s.minV = minV;
    s.maxV = maxV;
    slider.push_back( s );
}

void Interface::draw(){
    for (int i=0; i<slider.size(); i++) {
        Slider &s = slider[i];
        ofRectangle r = s.rect;
        ofFill();
        ofSetColor( 255, 255, 255 );
        ofDrawRectangle( r );
        ofSetColor( 0, 0, 0 );
        ofNoFill();
        ofDrawRectangle( r );
        ofFill();
        float w = ofMap( *s.value, s.minV, s.maxV, 0, r.width );
        ofDrawRectangle( r.x, r.y + 15, w, r.height - 15 );
        ofDrawBitmapString( s.title + " " + ofToString( *s.value, 2 ), r.x + 5, r.y + 12 );
    }
}

void Interface::mousePressed( int x, int y ){
    for (int i=0; i<slider.size(); i++) {
        Slider &s = slider[i];
        ofRectangle r = s.rect;
        if ( ofInRange( x, r.x, r.x + r.width ) && ofInRange( y, r.y, r.y + r.height ) ) {
            selected = i;
            *s.value = ofMap( x, r.x, r.x + r.width, s.minV, s.maxV, true );
        }
    }
}

void Interface::mouseDragged( int x, int y ){
    if ( selected >= 0 ) {
        Slider &s = slider[selected];
        ofRectangle r = s.rect;
        *s.value = ofMap( x, r.x, r.x + r.width, s.minV, s.maxV, true );
    }
}

void Interface::mouseReleased (int x, int y ){
    selected = -1;
}



void Interface::save( int index )
{
    vector<string> list;
    for (int i=0; i<slider.size(); i++) {
        list.push_back( ofToString( *slider[i].value ) );
    }
    string text = ofJoinString( list," " );
    string fileName = "preset" + ofToString( index ) + ".txt";
    ofBuffer buffer = ofBuffer( text );
    ofBufferToFile( fileName, buffer );
}

//--------------------------------------------------------------
void Interface::load( int index )
{
    string fileName = "preset" + ofToString( index ) + ".txt";
    string text = string( ofBufferFromFile( fileName ) );
    vector<string> list = ofSplitString( text, " " );
    
    if ( list.size() == slider.size() ) {
        for (int i=0; i<slider.size(); i++) {
            *slider[i].value = ofToFloat( list[i] );
        }
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    if (key == '1'){
        int mx = mouseX % w;
        int my = mouseY % h;
        trackHue1 = hue.getPixels()[my*w+mx];
    }
    
    if (key == '2'){
        int mx = mouseX % w;
        int my = mouseY % h;
        trackHue2 = hue.getPixels()[my*w+mx];
    }
    
    if (key == '3'){
        int mx = mouseX % w;
        int my = mouseY % h;
        trackHue3 = hue.getPixels()[my*w+mx];
    }
    
    switch (key){
        case ' ':
            drawCamera = ! drawCamera;
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
    if ( drawInterface ) {
        interf.mouseDragged( x, y );
    }
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
    if ( drawInterface ) {
        interf.mousePressed( x, y );
    }
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

