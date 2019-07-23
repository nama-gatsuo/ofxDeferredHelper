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
		Helper(const std::string& name = "deferred");
		void init(int w = ofGetWidth(), int h = ofGetHeight());
		void render(std::function<void(float, bool)> drawCall, ofCamera& cam, bool autoDraw = true);
		
		const ofFbo& getRenderedImage() const;
		const ofTexture& getTexture() const;

		void saveParams();
		void loadParams(const ofJson& json);

		// Load parameter with default name for click event
		void load();

		void debugDraw();
		void drawGui();

		// Draw lights. This fucntion should be called in frame buffer context
		void drawLights(float lds, bool isShadow);
		// Add a light to PointLightsPass. Mainly used for light object inheritance
		// TODO: Problem is that light objecs can't be flexible with scene.
		//       Helper must have all light pointers among all scene.
		void addLight(ofPtr<PointLight> light);

		Processor& getProcessor() {
			return processor;
		}
		const Processor& getProcessor() const {
			return processor;
		}
		const GBuffer& getGBuffer() const { return processor.getGBuffer(); }
		
		ofPtr<ofxDeferred::BgPass> bg;
		ofPtr<ofxDeferred::EdgePass> edge;
		ofPtr<ofxDeferred::SsaoPass> ssao;
		ofPtr<ofxDeferred::ShadowLightPass> shadow;
		ofPtr<ofxDeferred::PointLightPass> pointLight;
		ofPtr<ofxDeferred::DofPass> dof;
		ofPtr<ofxDeferred::BloomPass> bloom;

		void createGui();

	private:
		void createAllPasses();

		ofxDeferred::Processor processor;
		std::unordered_map<std::string, ofxGuiGroup> groups;

		ofxGuiGroup helperGroup;
		ofxButton saveButton;
		ofxButton loadButton;
		ofParameter<int> debugViewMode;
		
		std::string name;
	};
};
