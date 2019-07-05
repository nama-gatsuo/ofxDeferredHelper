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
		void init(int w = ofGetWidth(), int h = ofGetHeight(), bool loadParam = true, const std::string& name = "deferred");
		void render(std::function<void(float, bool)> drawCall, ofCamera& cam, bool autoDraw = true);
		const ofFbo& getRenderedImage() const;

		void save();
		void loadParams(const ofJson& json);
		void load() {
			loadParams(ofLoadJson("json/" + name + ".json"));
		}

		void debugDraw();
		void drawGui();
		void drawLights(float lds, bool isShadow);
		Processor& getProcessor() {
			return processor;
		}
		const Processor& getProcessor() const {
			return processor;
		}
		const GBuffer& getGBuffer() const { return processor.getGBuffer(); }
		void addLight(ofPtr<PointLight> light);
		
		ofPtr<ofxDeferred::BgPass> bg;
		ofPtr<ofxDeferred::EdgePass> edge;
		ofPtr<ofxDeferred::SsaoPass> ssao;
		ofPtr<ofxDeferred::ShadowLightPass> shadow;
		ofPtr<ofxDeferred::PointLightPass> pointLight;
		ofPtr<ofxDeferred::DofPass> dof;
		ofPtr<ofxDeferred::BloomPass> bloom;
	private:
		void createAllPasses();
		void createGui();

		ofxDeferred::Processor processor;
		std::unordered_map<std::string, ofxGuiGroup> groups;

		ofxGuiGroup helperGroup;
		ofxButton saveButton;
		ofxButton loadButton;
		ofParameter<int> debugViewMode;
		
		std::string name;
	};
};
