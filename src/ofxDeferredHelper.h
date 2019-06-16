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
		void drawLights(float lds, bool isShadow);
		const Processor& getProcessor() const {
			return processor;
		}
	private:

		void createPasses();
		void createGui();
		
		Processor processor;
		ofPtr<ShadowLightPass> shadow;
		ofPtr<PointLightPass> pointLight;
		ofPtr<DofPass> dof;
		ofPtr<BloomPass> bloom;
		std::vector<ofxGuiGroup> groups;

	};
};