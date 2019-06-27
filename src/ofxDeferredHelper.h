#pragma once

#include "ofxDeferredShading.h"
#include "ofxGui.h"
#include "ofTypes.h"
#include <functional>
#include "ofCamera.h"
#include "ofJson.h"

namespace ofxDeferred {
	class Helper {
	public:
		Helper();
		void init(int w = ofGetWidth(), int h = ofGetHeight());
		void render(std::function<void(float, bool)> drawCall, ofCamera& cam, bool autoDraw = true);
		const ofFbo& getRenderedImage() const;
		
		void save();
		void load();

		void debugDraw();
		void drawGui();
		//void load();

	private:
		void createAllPasses();
		void createPasses();
		void createGui();

		ofxDeferred::Processor processor;
		ofPtr<ofxDeferred::ShadowLightPass> shadow;
		ofPtr<ofxDeferred::PointLightPass> pointLight;
		ofPtr<ofxDeferred::BloomPass> bloom;
		ofPtr<ofxDeferred::DofPass> dof;
		std::unordered_map<std::string, ofxGuiGroup> groups;

		ofxGuiGroup helperGroup;
		ofxButton saveButton;
		ofxButton loadButton;
		ofParameter<int> debugViewMode;

		ofJson json;
	};
};