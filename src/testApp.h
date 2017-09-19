#pragma once

#include "MSAFluid.h"
#include "ParticleSystem.h"

#include "ofMain.h"
#include "ofxKinect.h"
#include "ofxOpenCv.h"
#include "ofxXmlSettings.h"
#include "ofxPostProcessing.h"

#define USE_GUI

#ifdef USE_GUI
#include "ofxSimpleGuiToo.h"
#endif

//#include "ofxXmlSettings.h"

#define stationaryForce 0.001f

using namespace cv;

class testApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();
		void exit();

        void drawUsers();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

        void loadSettings(void);
		void calcBlobColor(void);

        void fadeToColor(float r, float g, float b, float speed);
        void addToFluid(ofVec2f pos, ofVec2f vel, bool addColor, bool addForce);

        float                   colorMult;
        float                   velocityMult;
        int                     fluidCellsX;
        bool                    resizeFluid;
        bool                    drawFluid;
        bool                    drawParticles;

        msa::fluid::Solver      fluidSolver;
        msa::fluid::DrawerGl	fluidDrawer;
        ParticleSystem          particleSystem;
        ofVec2f                 pMouse;

		ofxKinect kinect;
		ofPixels depthPix, videoPix;
		ofMesh kinMesh;
		int step, zCut1, zCut2, ifx, dMax;
		int kinWidth, kinHeight;
		bool canDraw, bUsers, bBlobs, bFlow;
		bool point, wire, ball;
		unsigned long long elapsedTime, nextInt, prevInt;
		int timeInt;
		bool postPro;
		bool mirrorX, mirrorY;

        vector<ofImage> img;
		vector<ofShader> shaders;
		int shdInd, test;

        ofxCvGrayscaleImage imageDecimated, imageDecimated1, imageDecimated2;
        ofxCvGrayscaleImage depthCv, blurred, blurred1, blurred2, background, mask;
        ofxCvGrayscaleImage hue, sat, bri;
        ofxCvColorImage videoCv, hsb;
		ofxCvContourFinder contourFinder;
		vector<ofxCvBlob> blobs;
        vector<ofPolyline> contourPoly;
        vector<unsigned int> numPixels, redPixels, greenPixels, bluePixels, yellowPixels;

		ofxCvGrayscaleImage gray, gray1, gray2;	//Decimated grayscaled images
        ofxCvFloatImage flowX, flowY;		//Resulted optical flow in x and y axes

        float decimateB, decimateF;
        int wd, hd;   //Decimated size of input images

		ofxXmlSettings settings;

		ofxPostProcessing post;
};
