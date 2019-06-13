#pragma once

#include "ofxDeferredShading.h"
#include "ofxGui.h"
#include "ofTypes.h"
#include <functional>
#include "ofCamera.h"

namespace ofxDeferred {
	class Helper {
	public:
		Helper();
		void init(int w = ofGetWidth(), int h = ofGetHeight());
		void render(std::function<void(float, bool)> drawCall, ofCamera& cam, bool autoDraw = true);
		const ofFbo& getRenderedImage() const;
		void save();
		void drawGbuffer();
		void drawGui();
		void drawLights(bool isShadow);
	private:

		void createPasses();
		void createGui();

		ofxDeferred::Processor processor;
		ofPtr<ofxDeferred::ShadowLightPass> shadow;
		ofPtr<ofxDeferred::PointLightPass> pointLight;

		std::vector<ofxGuiGroup> groups;

	};
};