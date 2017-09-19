#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	// setup fluid stuff
	fluidSolver.setup(50, 50);
    fluidSolver.enableRGB(true).setFadeSpeed(0.002).setDeltaT(0.5).setVisc(0.00015).setColorDiffusion(0);
	fluidDrawer.setup(&fluidSolver);

	fluidCellsX			= 150;

	drawFluid			= true;
	drawParticles		= true;

	//ofSetFrameRate(30);
	//ofSetFullscreen(true);
	ofBackground(0, 0, 0);
	ofSetVerticalSync(true);

#ifdef USE_GUI
	gui.addSlider("fluidCellsX", fluidCellsX, 20, 400);
	gui.addButton("resizeFluid", resizeFluid);
  gui.addSlider("colorMult", colorMult, 0, 100);
  gui.addSlider("velocityMult", velocityMult, 0, 100);
	gui.addSlider("fs.viscocity", fluidSolver.viscocity, 0.0, 0.01);
	gui.addSlider("fs.colorDiffusion", fluidSolver.colorDiffusion, 0.0, 0.0003);
	gui.addSlider("fs.fadeSpeed", fluidSolver.fadeSpeed, 0.0, 0.1);
	gui.addSlider("fs.solverIterations", fluidSolver.solverIterations, 1, 50);
	gui.addSlider("fs.deltaT", fluidSolver.deltaT, 0.1, 5);
	gui.addComboBox("fd.drawMode", (int&)fluidDrawer.drawMode, msa::fluid::getDrawModeTitles());
	gui.addToggle("fs.doRGB", fluidSolver.doRGB);
	gui.addToggle("fs.doVorticityConfinement", fluidSolver.doVorticityConfinement);
	gui.addToggle("drawFluid", drawFluid);
	gui.addToggle("drawParticles", drawParticles);
	gui.addToggle("fs.wrapX", fluidSolver.wrap_x);
	gui.addToggle("fs.wrapY", fluidSolver.wrap_y);

	gui.currentPage().setXMLName("ofxMSAFluidSettings.xml");
  gui.loadFromXML();
	gui.setDefaultKeys(false);
	gui.setAutoSave(false);
    //gui.show();
    //gui.hide();
#endif

    windowResized(ofGetWidth(), ofGetHeight());		// force this at start (cos I don't think it is called)
	pMouse = msa::getWindowCenter();
	resizeFluid	= true;

	ofEnableAlphaBlending();
	ofSetBackgroundAuto(false);

    canDraw = false;
    bBlobs = true;
    bFlow = true;
    bUsers = true;
    point = true;
    wire = false;
    ball = false;

    loadSettings();


	// kinect stuff
    kinect.init(false, true, true);
    kinect.open();
    zCut1 = (zCut1 > 500) ? zCut1 : 500;
    kinect.setDepthClipping(zCut1 - 500, zCut2 + 500);
    kinect.setRegistration(true);
    kinect.setCameraTiltAngle(5);

    kinWidth = kinect.getWidth();
	kinHeight = kinect.getHeight();

    cout << kinWidth << endl;
    cout << kinHeight << endl;
    cout << step << " " << zCut1 << " " << zCut2 << endl;

    mirrorX = false;
    mirrorY = true;

    img.resize(5);
    img[0].loadImage("tex00.jpg");
    //img[1].loadImage("tex01.jpg");
    //img[1].loadImage("tex034.jpg");
    img[1].loadImage("acqua5sq.jpg");
    img[2].loadImage("tex02.jpg");
    img[3].loadImage("tex03.jpg");
    img[4].loadImage("tex04.jpg");

    shaders.resize(10);
    shaders[0].load("shaders/lines.vert", "shaders/lines.frag");
    shaders[1].load("shaders/lame.vert", "shaders/lame.frag");
    shaders[2].load("shaders/monjori.vert", "shaders/monjori.frag");
    shaders[3].load("shaders/water.vert", "shaders/water.frag");
    shaders[4].load("shaders/fire1.vert", "shaders/fire1.frag");
    shaders[5].load("shaders/blobbies.vert", "shaders/blobbies.frag");
    shaders[6].load("shaders/normalmap.vert", "shaders/normalmap.frag");
    shaders[7].load("shaders/lame.vert", "shaders/lame.frag");
    shaders[8].load("shaders/plasma.vert", "shaders/plasma.frag");
    shaders[9].load("shaders/lame2.vert", "shaders/lame2.frag");
    //shaders[1].load("shaders/water121212.vert", "shaders/water121212.frag");
    //shaders[2].load("shaders/clouds-tunnel.vert", "shaders/clouds-tunnel.frag");
    shdInd = 0;

    depthPix.allocate(kinWidth, kinHeight, OF_IMAGE_GRAYSCALE);

    depthCv.allocate(kinWidth, kinHeight);
    videoCv.allocate(kinWidth, kinHeight);
    hsb.allocate(kinWidth, kinHeight);
    hue.allocate(kinWidth, kinHeight);
    sat.allocate(kinWidth, kinHeight);
    bri.allocate(kinWidth, kinHeight);

    //Decimate images
    decimateF = 0.25;   // flow
    decimateB = 0.5;    // blob

    imageDecimated.allocate(kinWidth * decimateB, kinHeight * decimateB);
    imageDecimated1.allocate(kinWidth * decimateF, kinHeight * decimateF);
    imageDecimated2.allocate(kinWidth * decimateF, kinHeight * decimateF);

    // PostProcessing
    post.init(ofGetWidth(), ofGetHeight());
		post.createPass<FxaaPass>()->setEnabled(false);
    post.createPass<BloomPass>()->setEnabled(false);
    post.createPass<DofPass>()->setEnabled(false);
    post.createPass<ToonPass>()->setEnabled(false);
    post.createPass<BleachBypassPass>()->setEnabled(false);
    post.createPass<EdgePass>()->setEnabled(false);
    post.setFlip(false);
    postPro = false;

    nextInt = ofGetElapsedTimeMillis();
    ifx = 0;
    dMax = 100;
}

void testApp::loadSettings(void) {
    settings.loadFile("settings.xml");
    step = settings.getValue("settings:step", 4);
    zCut1 = settings.getValue("settings:zCut1", 500);
    zCut2 = settings.getValue("settings:zCut2", 1000);
    timeInt = settings.getValue("settings:timeInt", 10);
    test = settings.getValue("settings:test", 0);
    mirrorX = settings.getValue("settings:mirrorX", 0) == 0 ? false : true;
    mirrorY = settings.getValue("settings:mirrorY", 1) == 0 ? false : true;
}

void testApp::fadeToColor(float r, float g, float b, float speed) {
    glColor4f(r, g, b, speed);
	ofRect(0, 0, ofGetWidth(), ofGetHeight());
}

// add force and dye to fluid, and create particles
void testApp::addToFluid(ofVec2f pos, ofVec2f vel, bool addColor, bool addForce) {
    float speed = vel.x * vel.x  + vel.y * vel.y * msa::getWindowAspectRatio() * msa::getWindowAspectRatio();    // balance the x and y components of speed with the screen aspect ratio
    if(speed > 0) {
		pos.x = ofClamp(pos.x, 0.0f, 1.0f);
		pos.y = ofClamp(pos.y, 0.0f, 1.0f);

        int index = fluidSolver.getIndexForPos(pos);

		if(addColor) {
//			Color drawColor(CM_HSV, (getElapsedFrames() % 360) / 360.0f, 1, 1);
			ofColor drawColor;
			drawColor.setHsb((ofGetFrameNum() % 255), 255, 255);

			fluidSolver.addColorAtIndex(index, drawColor * colorMult);

			if(drawParticles)
				particleSystem.addParticles(pos * ofVec2f(ofGetWindowSize()), 10);
		}

		if(addForce)
			fluidSolver.addForceAtIndex(index, vel * velocityMult);

    }
}

//--------------------------------------------------------------
void testApp::exit(){
    cout << "exit" << endl;

    ofSoundShutdown();

    kinect.close();
    kinect.clear();

    cout << "exit" << endl;
}

//--------------------------------------------------------------
void testApp::update(){

    // fluid stuff
    if(resizeFluid) {
		fluidSolver.setSize(fluidCellsX, fluidCellsX / msa::getWindowAspectRatio());
		fluidDrawer.setup(&fluidSolver);
		resizeFluid = false;
	}

    // fx sequence
    if (test == 0 && kinect.isConnected()) {
        elapsedTime = ofGetElapsedTimeMillis();
        if (elapsedTime >= nextInt) {
            prevInt = nextInt;
            nextInt = elapsedTime + 1000*timeInt;

            switch (ifx) {
                case 0:   // shader 0
                    cout << "000 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawSpeed);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(false);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(false);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(false);   // EdgePass
                    postPro = false;
                    shdInd = 0;
                    break;
                case 1:
                    cout << "111 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawMotion);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(true);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(true);   // ToonPass
                    post[4]->setEnabled(true);   // BleachBypassPass
                    post[5]->setEnabled(true);   // EdgePass
                    postPro = true;
                    shdInd = 0;
                    break;
                case 2:
                    cout << "222 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawMotion);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(false);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(false);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(false);   // EdgePass
                    postPro = false;
                    shdInd = 0;
                    break;
                case 3:
                    cout << "333 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawColor);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(false);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(false);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(false);   // EdgePass
                    postPro = false;
                    shdInd = 0;
                    break;
                case 4:
                    cout << "444 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawMotion);
                    drawFluid = true;
                    drawParticles = false;
                    post[0]->setEnabled(true);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(true);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(false);   // EdgePass
                    postPro = true;
                    shdInd = 0;
                    break;
                case 5:
                    cout << "555 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawVectors);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(false);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(false);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(false);   // EdgePass
                    postPro = false;
                    shdInd = 0;
                    break;
                case 6:
                    cout << "666 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawVectors);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(false);   // FxaaPass
                    post[1]->setEnabled(true);   // BloomPass
                    post[2]->setEnabled(true);   // DofPass
                    post[3]->setEnabled(true);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(true);   // EdgePass
                    postPro = true;
                    shdInd = 0;
                    break;
                case 7:   // shader 1
                    cout << "000 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawSpeed);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(false);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(false);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(false);   // EdgePass
                    postPro = false;
                    shdInd = 1;
                    break;
                case 8:
                    cout << "111 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawMotion);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(true);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(true);   // ToonPass
                    post[4]->setEnabled(true);   // BleachBypassPass
                    post[5]->setEnabled(true);   // EdgePass
                    postPro = true;
                    shdInd = 1;
                    break;
                case 9:
                    cout << "222 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawMotion);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(false);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(false);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(false);   // EdgePass
                    postPro = false;
                    shdInd = 1;
                    break;
                case 10:
                    cout << "333 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawColor);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(false);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(false);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(false);   // EdgePass
                    postPro = false;
                    shdInd = 1;
                    break;
                case 11:
                    cout << "444 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawMotion);
                    drawFluid = true;
                    drawParticles = false;
                    post[0]->setEnabled(true);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(true);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(false);   // EdgePass
                    postPro = true;
                    shdInd = 1;
                    break;
                case 12:
                    cout << "555 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawVectors);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(false);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(false);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(false);   // EdgePass
                    postPro = false;
                    shdInd = 1;
                    break;
                case 13:
                    cout << "666 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawVectors);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(false);   // FxaaPass
                    post[1]->setEnabled(true);   // BloomPass
                    post[2]->setEnabled(true);   // DofPass
                    post[3]->setEnabled(true);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(true);   // EdgePass
                    postPro = true;
                    shdInd = 1;
                    break;
                case 14:   // shader 2
                    cout << "000 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawSpeed);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(false);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(false);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(false);   // EdgePass
                    postPro = false;
                    shdInd = 2;
                    break;
                case 15:
                    cout << "111 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawMotion);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(true);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(true);   // ToonPass
                    post[4]->setEnabled(true);   // BleachBypassPass
                    post[5]->setEnabled(true);   // EdgePass
                    postPro = true;
                    shdInd = 2;
                    break;
                case 16:
                    cout << "222 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawMotion);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(false);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(false);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(false);   // EdgePass
                    postPro = false;
                    shdInd = 2;
                    break;
                case 17:
                    cout << "333 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawColor);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(false);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(false);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(false);   // EdgePass
                    postPro = false;
                    shdInd = 2;
                    break;
                case 18:
                    cout << "444 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawMotion);
                    drawFluid = true;
                    drawParticles = false;
                    post[0]->setEnabled(true);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(true);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(false);   // EdgePass
                    postPro = true;
                    shdInd = 2;
                    break;
                case 19:
                    cout << "555 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawVectors);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(false);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(false);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(false);   // EdgePass
                    postPro = false;
                    shdInd = 2;
                    break;
                case 20:
                    cout << "666 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawVectors);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(false);   // FxaaPass
                    post[1]->setEnabled(true);   // BloomPass
                    post[2]->setEnabled(true);   // DofPass
                    post[3]->setEnabled(true);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(true);   // EdgePass
                    postPro = true;
                    shdInd = 2;
                    break;
                case 21:   // shader 3
                    cout << "000 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawSpeed);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(false);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(false);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(false);   // EdgePass
                    postPro = false;
                    shdInd = 3;
                    break;
                case 22:
                    cout << "111 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawMotion);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(true);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(true);   // ToonPass
                    post[4]->setEnabled(true);   // BleachBypassPass
                    post[5]->setEnabled(true);   // EdgePass
                    postPro = true;
                    shdInd = 3;
                    break;
                case 23:
                    cout << "222 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawMotion);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(false);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(false);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(false);   // EdgePass
                    postPro = false;
                    shdInd = 3;
                    break;
                case 24:
                    cout << "333 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawColor);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(false);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(false);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(false);   // EdgePass
                    postPro = false;
                    shdInd = 3;
                    break;
                case 25:
                    cout << "444 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawMotion);
                    drawFluid = true;
                    drawParticles = false;
                    post[0]->setEnabled(true);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(true);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(false);   // EdgePass
                    postPro = true;
                    shdInd = 3;
                    break;
                case 26:
                    cout << "555 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawVectors);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(false);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(false);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(false);   // EdgePass
                    postPro = false;
                    shdInd = 3;
                    break;
                case 27:
                    cout << "666 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawVectors);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(false);   // FxaaPass
                    post[1]->setEnabled(true);   // BloomPass
                    post[2]->setEnabled(true);   // DofPass
                    post[3]->setEnabled(true);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(true);   // EdgePass
                    postPro = true;
                    shdInd = 3;
                    break;
                case 28:   // shader 4
                    cout << "000 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawSpeed);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(false);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(false);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(false);   // EdgePass
                    postPro = false;
                    shdInd = 4;
                    break;
                case 29:
                    cout << "111 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawMotion);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(true);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(true);   // ToonPass
                    post[4]->setEnabled(true);   // BleachBypassPass
                    post[5]->setEnabled(true);   // EdgePass
                    postPro = true;
                    shdInd = 4;
                    break;
                case 30:
                    cout << "222 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawMotion);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(false);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(false);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(false);   // EdgePass
                    postPro = false;
                    shdInd = 4;
                    break;
                case 31:
                    cout << "333 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawColor);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(false);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(false);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(false);   // EdgePass
                    postPro = false;
                    shdInd = 4;
                    break;
                case 32:
                    cout << "444 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawMotion);
                    drawFluid = true;
                    drawParticles = false;
                    post[0]->setEnabled(true);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(true);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(false);   // EdgePass
                    postPro = true;
                    shdInd = 4;
                    break;
                case 33:
                    cout << "555 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawVectors);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(false);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(false);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(false);   // EdgePass
                    postPro = false;
                    shdInd = 4;
                    break;
                case 34:
                    cout << "666 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawVectors);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(false);   // FxaaPass
                    post[1]->setEnabled(true);   // BloomPass
                    post[2]->setEnabled(true);   // DofPass
                    post[3]->setEnabled(true);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(true);   // EdgePass
                    postPro = true;
                    shdInd = 4;
                    break;
                case 35:   // shader 5
                    cout << "000 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawSpeed);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(false);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(false);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(false);   // EdgePass
                    postPro = false;
                    shdInd = 5;
                    break;
                case 36:
                    cout << "111 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawMotion);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(true);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(true);   // ToonPass
                    post[4]->setEnabled(true);   // BleachBypassPass
                    post[5]->setEnabled(true);   // EdgePass
                    postPro = true;
                    shdInd = 5;
                    break;
                case 37:
                    cout << "222 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawMotion);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(false);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(false);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(false);   // EdgePass
                    postPro = false;
                    shdInd = 5;
                    break;
                case 38:
                    cout << "333 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawColor);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(false);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(false);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(false);   // EdgePass
                    postPro = false;
                    shdInd = 5;
                    break;
                case 39:
                    cout << "444 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawMotion);
                    drawFluid = true;
                    drawParticles = false;
                    post[0]->setEnabled(true);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(true);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(false);   // EdgePass
                    postPro = true;
                    shdInd = 5;
                    break;
                case 40:
                    cout << "555 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawVectors);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(false);   // FxaaPass
                    post[1]->setEnabled(false);   // BloomPass
                    post[2]->setEnabled(false);   // DofPass
                    post[3]->setEnabled(false);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(false);   // EdgePass
                    postPro = false;
                    shdInd = 5;
                    break;
                case 41:
                    cout << "666 " << elapsedTime/1000 << " " << (nextInt - prevInt)/1000 << endl;
                    canDraw = false;
                    fluidDrawer.setDrawMode(msa::fluid::kDrawVectors);
                    drawFluid = true;
                    drawParticles = true;
                    post[0]->setEnabled(false);   // FxaaPass
                    post[1]->setEnabled(true);   // BloomPass
                    post[2]->setEnabled(true);   // DofPass
                    post[3]->setEnabled(true);   // ToonPass
                    post[4]->setEnabled(false);   // BleachBypassPass
                    post[5]->setEnabled(true);   // EdgePass
                    postPro = true;
                    shdInd = 5;
                    break;
            }

            ifx++;
            if (ifx == 42) {
                ifx = 0;
            }
        }
    }
    // kinect stuff
    kinect.update();
    if (kinect.isFrameNew()) {
        canDraw = true;
        kinMesh.clear();
        kinMesh.setMode(OF_PRIMITIVE_TRIANGLES);
        depthPix = kinect.getDepthPixels();
        //depthPix = kinect.getDepthPixelsRef();
        videoPix = kinect.getPixels();
				//videoPix = kinect.getPixelsRef();
        int i = 0;
        int i3 = 0;
        for (int y = 0; y < kinHeight; y++) {
            for (int x = 0; x < kinWidth; x++) {
                float dist = kinect.getDistanceAt(x, y);
                ofColor kinCol = kinect.getColorAt(x, y);
                ofPoint kinPoint = ofPoint(x, y, zCut1);
                if (dist > zCut1 && dist < zCut2) {
                    depthPix[i] = 255;
                    if ((x % step) == 0 && (y % step) == 0) {
                        kinMesh.addVertex(kinPoint);
                        kinMesh.addColor(kinCol);
                    }
                } else {
                    depthPix[i] = 0;
                    videoPix[i3] = 0;
                    videoPix[i3 + 1] = 0;
                    videoPix[i3 + 2] = 0;
                    if ((x % step) == 0 && (y % step) == 0) {
                        kinPoint.z = zCut2;
                        kinMesh.addVertex(kinPoint);
                        kinMesh.addColor(kinCol);
                    }
                }
                i++;
                i3 += 3;
            }
        }

        for (int y = 0; y < kinHeight/step - 1; y += 1) {
            for (int x = 0; x < kinWidth/step - 1; x += 1) {
                ofVec3f va = kinMesh.getVertex(x+y*(kinWidth/step));
                ofVec3f vb = kinMesh.getVertex((x+1)+y*(kinWidth/step));
                ofVec3f vc = kinMesh.getVertex((x+1)+(y+1)*(kinWidth/step));
                ofVec3f vd = kinMesh.getVertex(x+(y+1)*(kinWidth/step));
                if (va.z != zCut2 && vb.z != zCut2 && vc.z != zCut2 && vd.z != zCut2) {
                    if (va.distanceSquared(vb) < dMax && vb.distanceSquared(vc) < dMax && vc.distanceSquared(vd) < dMax && vd.distanceSquared(va) < dMax) {
                        if ((va.x != 0 || va.y != 0) && (vb.x != 0 || vb.y != 0) && (vc.x != 0 || vc.y != 0) && (vd.x != 0 || vd.y != 0)) {
                            kinMesh.addIndex(x+y*(kinWidth/step));			    // 0
                            kinMesh.addIndex((x+1)+y*(kinWidth/step));			// 1
                            kinMesh.addIndex(x+(y+1)*(kinWidth/step));			// 10

                            kinMesh.addIndex((x+1)+y*(kinWidth/step));			// 1
                            kinMesh.addIndex((x+1)+(y+1)*(kinWidth/step));		// 11
                            kinMesh.addIndex(x+(y+1)*(kinWidth/step));			// 10
                        }
                    }
                }
            }
        }

        depthCv.setFromPixels(depthPix);
        depthCv.mirror(mirrorX, mirrorY);
        videoCv.setFromPixels(videoPix);
        videoCv.mirror(mirrorX, mirrorY);

        hsb = videoCv;
        //convert to hsb
        hsb.convertRgbToHsv();
        //store the three channels as grayscale images
        hsb.convertToGrayscalePlanarImages(hue, sat, bri);

        //depthCv.threshold(128);
        blurred1 = depthCv;
        blurred1.erode();
        blurred1.erode();
        //blurred1.erode();
        //blurred1.erode();
        blurred1.blurGaussian(9);
        blurred1.threshold(128);
        blurred1.dilate();
        blurred = blurred1;
        blurred.blurGaussian(9);
        blurred1.blurHeavily();

        if (!blurred2.bAllocated) {
            blurred2 = blurred1;
        }

        //High-quality resize
        imageDecimated.scaleIntoMe( blurred, CV_INTER_AREA );
        gray = imageDecimated;
        gray.blurGaussian(9);

        //High-quality resize
        imageDecimated1.scaleIntoMe( blurred1, CV_INTER_AREA );
        gray1 = imageDecimated1;
        gray1.blurGaussian(9);

        //High-quality resize
        imageDecimated2.scaleIntoMe( blurred2, CV_INTER_AREA );
        gray2 = imageDecimated2;
        gray2.blurGaussian(9);

        wd = gray1.width;
        hd = gray1.height;

        // optical flow ----------------------------------------
        if (bFlow) {
            Mat img1(gray1.getCvImage());  //Create OpenCV images
            Mat img2(gray2.getCvImage());
            Mat flow;                        //Image for flow
            //Computing optical flow
            //calcOpticalFlowFarneback(img2, img1, flow, 0.7, 3, 11, 5, 5, 1.1, 0);
            calcOpticalFlowFarneback(img2, img1, flow, 0.5, 3, 15, 3, 5, 1.1, 0);
            //Split flow into separate images
            vector<Mat> flowPlanes;
            split(flow, flowPlanes);
            //Copy float planes to ofxCv images flowX and flowY
            IplImage iplX(flowPlanes[0]);
            flowX = &iplX;
            IplImage iplY(flowPlanes[1]);
            flowY = &iplY;
        }

        // blobs -----------------------------------------------
        if (bBlobs) {
            if (!background.bAllocated) {
                background.allocate(kinWidth * decimateB, kinHeight * decimateB);
                background.set(0);
            }

            mask = gray;
            mask -= background;
            mask.threshold(40);

            contourFinder.findContours(mask, 300, 100000, 20, false);

            blobs = contourFinder.blobs;
            int n = blobs.size();
            contourPoly.clear();
            for (int j = 0; j < n; j++) {
                if(blobs[j].pts.size() > 5){
                    ofPolyline tempPoly;
                    tempPoly.addVertices(blobs[j].pts);
                    tempPoly.setClosed(true);
                    ofPolyline smoothPoly;
                    smoothPoly = tempPoly.getSmoothed(2, 0.5);
                    if(!smoothPoly.isClosed()){
                        smoothPoly.close();
                    }
                    contourPoly.push_back(smoothPoly);
                }
            }
        }

        blurred2 = blurred1;

        // kinect - fluid interactivity
        //Optical flow
        float *flowXPixels = flowX.getPixelsAsFloats();
        float *flowYPixels = flowY.getPixelsAsFloats();
				int fac = (int)(1.0 / decimateF);
				int j = 0;
        for (int y = 0; y < hd; y += step) {
            for (int x = 0; x < wd; x += step) {
                float fx = flowXPixels[ x + wd * y ];
                float fy = flowYPixels[ x + wd * y ];
                //Draw only long vectors
                float fm = fabs(fx) + fabs(fy);

                bool inside = false;
                int n = contourPoly.size();
                for (int j = 0; j < n; j++) {
                    inside = contourPoly[j].size() > 5 && contourPoly[j].inside(x*decimateB/decimateF, y*decimateB/decimateF);
                    if (inside)
                        break;
                }

								float dpt = 5.0 * (1.0 + 5.0 * kinect.getDistanceAt(x * fac, y * fac) / zCut2);

                if (inside && fm > 2.0 && fm < 25.0) {
                    //addToFluid(ofVec2f(float(x) / float(wd), float(y) / float(hd)), ofVec2f(fx / 500.0, fy / 500.0), true, true);
										addToFluid(ofVec2f(float(x) / float(wd), float(y) / float(hd)), ofVec2f(dpt * fx / 5000.0, dpt * fy / 5000.0), true, true);
		            }

								//cout << dpt << '\n';
								j += fac;

            }
        }
    }

    fluidSolver.update();
}

//--------------------------------------------------------------
void testApp::draw(){

    float w = ofGetWidth();
    float h = ofGetHeight();
    float sx = w / float(kinWidth);
    float sy = h / float(kinHeight);

    //depthCv.draw(0, 0, w, h);
    //videoCv.draw(0, 0, w, h);
    //hue.draw(0, 0, w, h);
    //bri.draw(0, 0, w, h);

    if (postPro) {
        post.begin();
    }

    if(drawFluid) {
        ofClear(0);
	glColor3f(1, 1, 1);
	fluidDrawer.draw(0, 0, ofGetWidth(), ofGetHeight());
	} else {
        fadeToColor(0, 0, 0, 0.01);
	}
	if(drawParticles) {
	particleSystem.updateAndDraw(fluidSolver, ofGetWindowSize(), drawFluid);
	}

    if (postPro) {
        post.end(false);
				post.setFlip(true);
        post.draw(0, 0, w, h);
    }

#ifdef USE_GUI
	//gui.draw();

	ofPushStyle();

	glDisable(GL_DEPTH_TEST);

	ofSetLineWidth(1);

	glDisableClientState(GL_COLOR_ARRAY);

	ofPopStyle();
#endif

    if (canDraw && bUsers) {
        drawUsers();

        ofPushMatrix();
        //ofScale(sx, sy*1.2);

        if (mirrorX) {
            sy = -sy;
        } else {
            h = 0;
        }
        if (mirrorY) {
            sx = -sx;
        } else {
            w = 0;
        }
        ofTranslate(w, h, zCut1);
        ofScale(sx, sy*1.2, -1);

        ofEnableAntiAliasing();
        kinMesh.drawWireframe();
        ofDisableAntiAliasing();

        ofPopMatrix();
    }

    //string msg = msaMode + (ppMode5 ? " 5 " : "") + (ppMode6 ? " 6 " : "") + (ppMode7 ? " 7 " : "") + (ppMode8 ? " 8 " : "");
    //ofDrawBitmapString(msg, 50, 50);
}

void testApp::drawUsers(){
    //ofSetColor(255, 255, 255);

    float w = ofGetWidth();
    float h = ofGetHeight();

    // Blobs
    if (bBlobs) {
        ofPushMatrix();
        float sx = w / float(kinWidth * decimateB);
        float sy = h / float(kinHeight * decimateB);
        ofScale(sx, sy*1.2);
        //ofScale(sx, sy);
        int n = contourPoly.size();

        ofFill();
        ofSetLineWidth(1);
        for (int i = 0; i < n; i++) {
            //contourPoly[i].draw();

            ofEnableNormalizedTexCoords();

            ofPoint center = contourPoly[i].getCentroid2D();
            //cout << center.x << " " << center.y << endl;
            shaders[shdInd].begin();
            shaders[shdInd].setUniform1f("time", ofGetElapsedTimef() * 1.0);
            shaders[shdInd].setUniform1f("alp", 0.7);
            shaders[shdInd].setUniform2f("mouse", center.x / 320.0, center.y / 240.0);
            shaders[shdInd].setUniform2f("resolution", ofGetWidth(), ofGetHeight());
            if (shdInd == 1) {
                shaders[shdInd].setUniformTexture("colorMap", img[shdInd], 0);
            }

            ofBeginShape();
            ofSetColor(255, 255, 255, 127);
            for(int j = 0; j < contourPoly[i].size(); j++) {
                ofVertex(contourPoly[i][j].x, contourPoly[i][j].y);
            }
            ofEndShape(true);

            shaders[shdInd].end();

            ofDisableNormalizedTexCoords();
        }
        ofPopMatrix();
    }

    //if (bFlow) {
    if (false) {
        //Flow image
        ofPushMatrix();
        float sx = w / float(wd);
        float sy = h / float(hd);
        //ofScale(sx, sy*1.2);
        ofScale(sx, sy);

        //Optical flow
        float *flowXPixels = flowX.getPixelsAsFloats();
        float *flowYPixels = flowY.getPixelsAsFloats();
        ofSetColor(255, 255, 255, 127);
        ofNoFill();
        for (int y = 0; y < hd; y += step) {
            for (int x = 0; x < wd; x += step) {
                float fx = flowXPixels[ x + wd * y ];
                float fy = flowYPixels[ x + wd * y ];
                //Draw only long vectors
                float fm = fabs(fx) + fabs(fy);

                bool inside = false;
                int n = contourPoly.size();
                for (int i = 0; i < n; i++) {
                    inside = contourPoly[i].size() > 5 && contourPoly[i].inside(x*decimateB/decimateF, y*decimateB/decimateF) && fm < 25.0;
                    if (inside)
                        break;
                }

                shaders[shdInd].begin();
                shaders[shdInd].setUniform1f("time", ofGetElapsedTimef() * 1.0);
                shaders[shdInd].setUniform1f("alp", 0.7);
                shaders[shdInd].setUniform2f("mouse", (mouseX - ofGetWidth()/2) * 0.001, (ofGetHeight()/2 - mouseY) * 0.005);
                shaders[shdInd].setUniform2f("resolution", ofGetWidth(), ofGetHeight());

                ofSetColor(255, 255, 255);
                ofSetLineWidth(1);
                ofFill();

                if (inside && point) {
                    ofCircle(x, y, 1);
                } else if (inside && wire) {
                    ofRect(x - (1.0 + fx / 2.0), y - (1.0 + fy / 2.0), 2.0 + fx, 2.0 + fy);
                } else if (inside && ball) {
                    ofCircle(x, y, 0.5 + fm / 3.0);
                }
                shaders[shdInd].end();
            }
        }
        ofPopMatrix();
    }
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    switch(key) {
		case 'f':
			ofToggleFullscreen();
			break;
    case '0':
      postPro = ! postPro;
			break;
    case '1':
      fluidDrawer.setDrawMode(msa::fluid::kDrawColor);
			break;
		case '2':
      fluidDrawer.setDrawMode(msa::fluid::kDrawMotion);
			break;
    case '3':
      fluidDrawer.setDrawMode(msa::fluid::kDrawSpeed);
			break;
    case '4':
      fluidDrawer.setDrawMode(msa::fluid::kDrawVectors);
			break;
    case '5':
			//post[0]->setEnabled(!post[0]->getEnabled());
			post[1]->setEnabled(!post[1]->getEnabled());
			post[2]->setEnabled(!post[2]->getEnabled());
			break;
    case '6':
			//post[0]->setEnabled(!post[0]->getEnabled());
			post[3]->setEnabled(!post[3]->getEnabled());
			break;
    case '7':
			post[4]->setEnabled(!post[4]->getEnabled());
			break;
    case '8':
			post[5]->setEnabled(!post[5]->getEnabled());
			break;
    case '9':
			post[0]->setEnabled(!post[0]->getEnabled());
			break;
		case 'd':
			drawFluid ^= true;
			break;
		case 'p':
			drawParticles ^= true;
			break;
    case 'u':
      bUsers = ! bUsers;
			break;
		case 'R':
			fluidSolver.reset();
			loadSettings();
			nextInt = ofGetElapsedTimeMillis();
			ifx = 0;
			break;
	}
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){
    ofVec2f eventPos = ofVec2f(x, y);
	ofVec2f mouseNorm = ofVec2f(eventPos) / ofGetWindowSize();
	ofVec2f mouseVel = ofVec2f(eventPos - pMouse) / ofGetWindowSize();
	addToFluid(mouseNorm, mouseVel, true, true);
	pMouse = eventPos;
	//cout << mouseVel.x << " " << mouseVel.y << endl;
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	ofVec2f eventPos = ofVec2f(x, y);
	ofVec2f mouseNorm = ofVec2f(eventPos) / ofGetWindowSize();
	ofVec2f mouseVel = ofVec2f(eventPos - pMouse) / ofGetWindowSize();
	addToFluid(mouseNorm, mouseVel, false, true);
	pMouse = eventPos;
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
    float w = ofGetWidth();
    float h = ofGetHeight();

    int mx = (float)x * (float)kinWidth / w;
    int my = (float)y * (float)kinHeight / h;

    //get hue value on mouse position
    int findHue = hue.getPixels()[my*kinWidth+mx];
    cout << findHue << endl;
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){

}
