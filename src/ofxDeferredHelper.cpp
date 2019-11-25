#include "ofxDeferredHelper.h"
#include "BaseObject.h"

namespace ofxDeferred {

	Helper::Helper(const std::string& name) : name(name) {}

	void Helper::init(int w, int h) {
		processor = std::make_shared<Processor>();
		processor->init(w, h);
		createAllPasses();
		createGui();
	}

	void Helper::render(std::function<void(const ofxDeferred::RenderInfo& mode)> drawCall, ofCamera& cam, bool autoDraw) {
		if (shadow && shadow->getEnabled()) {
			shadow->beginShadowMap(cam);
			ofxDeferred::RenderInfo mode{
				shadow->getLinearScalar(), true, 
				glm::vec4(0), glm::mat4() };
			drawCall(mode);
			drawLights(mode);
			shadow->endShadowMap();
		}

		
		float lds = 1. / (cam.getFarClip() - cam.getNearClip());
		ofxDeferred::RenderInfo mode{ lds, false, glm::vec4(0, -1, 0, 100000), glm::inverse(cam.getModelViewMatrix()) };
		processor->begin(cam, true);
		drawCall(mode);
		drawLights(mode);
		processor->end(autoDraw);
	}

	const ofFbo& Helper::getRenderedImage() const {
		return processor->getFbo();
	}

	const ofTexture& Helper::getTexture() const {
		return getRenderedImage().getTexture();
	}

	void Helper::saveParams() {
	
		const std::string path = "json/renderers/" + name + ".json";

		ofJson json = ofLoadJson(path);

		// Create a child object of json and add ParameterGroup to it
		ofParameterGroup g;
		g.setName("deferred");
		for (auto& gr : groups) {
			g.add(gr.second.getParameter());
		}
	
		ofJson cj;
		ofSerialize(cj, g); // cj << g
		json["deferred"] = cj["deferred"];
	
		ofSavePrettyJson(path, json);
	}

	void Helper::loadParams(const ofJson& json) {
		if (!json.is_null()) {
			const auto& j = json["deferred"];
			for (auto& it = j.cbegin(); it != j.cend(); ++it) {
				ofJson cj;
				cj[it.key()] = *it;
				groups[it.key()].loadFrom(cj);
			}
		} else {
			ofLogError("ofxDeferred::Helper") << "no json file found";
		}
	}

	void Helper::load() {
		auto& json = ofLoadJson("json/renderers/" + name + ".json");
		if (json.is_null()) ofLogError("ofxDeferredHelper") << "No json found: " << name;
		else loadParams(json);
	}

	void Helper::debugDraw() {
		glm::vec2 s(ofGetViewportWidth() * 0.25, ofGetViewportHeight() * 0.25);
		glm::vec2 p(0, ofGetHeight() - s.y);

		switch (debugViewMode.get()) {
		case 1: {
			processor->debugDraw();
		}break;
		case 2:{
			if (shadow) shadow->debugDraw(p, s);
		}break;
		case 3: {
			if (dof) dof->debugDraw(p, s);
		}break;
		case 4: {
			if (bloom) bloom->debugDraw(p, s);
		}break;

		default: break;
		}

	}

	void Helper::drawGui() {
		ofPushStyle();
		ofEnableAlphaBlending();
		for (auto& g : groups) {
			g.second.draw();
		}
		helperGroup.draw();
		ofPopStyle();
	}

	void Helper::drawLights(const ofxDeferred::RenderInfo& mode) {
		if (pointLight->getEnabled()) pointLight->drawLights(mode);
	}

	void Helper::addLight(ofPtr<PointLight> light) {
		pointLight->addLight(light);
	}

	void Helper::createAllPasses() {
		bg = processor->createPass<ofxDeferred::BgPass>();
		bg->begin();
		ofClear(10, 12, 24, 255);
		bg->end();

		edge = processor->createPass<ofxDeferred::EdgePass>();
		edge->setUseReadColor(true);
		edge->setEdgeColor(ofFloatColor(0.));
		ssao = processor->createPass<ofxDeferred::SsaoPass>();
		shadow = processor->createPass<ofxDeferred::ShadowLightPass>();
		shadow->setPosition(glm::vec3(100., 200., 100.));
		shadow->lookAt(glm::vec3(0.));
		pointLight = processor->createPass<ofxDeferred::PointLightPass>();
		for (int i = 0; i < 4; i++) {
			pointLight->addLight();
		}
		fog = processor->createPass<FogPass>();
		dof = processor->createPass<ofxDeferred::DofPass>();
		bloom = processor->createPass<ofxDeferred::BloomPass>();
	}

	void Helper::createGui() {

		float heightSum = 10.;
		float widthSum = 10.;
		const float dw = 240.;
		const float colorOffset = 280.;

		for (int i = 0; i < processor->size(); i++) {
			const std::string& name = (*processor)[i]->getName();

			groups[name].setDefaultWidth(dw);
			groups[name].setBackgroundColor(ofFloatColor(0., 0.5));
			groups[name].setDefaultBackgroundColor(ofFloatColor(0., 0.5));
			groups[name].setHeaderBackgroundColor(ofFloatColor(0.6, 0.3, 0.8, 0.5));
			groups[name].setDefaultHeaderBackgroundColor(ofFloatColor(0.0, 0.5));
			groups[name].setFillColor(ofFloatColor(0.3, 0.3, 0.6, 0.5));
			groups[name].setDefaultFillColor(ofFloatColor(0.3, 0.3, 0.6, 0.5));
			groups[name].setBorderColor(ofFloatColor(0.1, 0.5));
			groups[name].setDefaultBorderColor(ofFloatColor(0.1, 0.5));
			
			groups[name].setup((*processor)[i]->getParameters());
			groups[name].setName(name);

			float h = groups[name].getHeight();
			float hOffset = 10.;
			if (name == RenderPassRegistry::Bg ||
				name == RenderPassRegistry::Edge ||
				name == RenderPassRegistry::PointLight ||
				name == RenderPassRegistry::ShadowLight) {

				hOffset = colorOffset;
			}

			if (heightSum + h + hOffset > ofGetViewportHeight()) {
				// line break
				widthSum += dw + 10.;
				groups[name].setPosition(widthSum, 10.);
				heightSum = h + 10.;
			} else {
				groups[name].setPosition(widthSum, heightSum);
				heightSum += h;
			}

			if (name == RenderPassRegistry::PointLight) {
				groups[name].minimizeAll();
			}

			heightSum += hOffset;
		}

		createHelperGui();
		helperGroup.setPosition(widthSum, heightSum);

		// Load parameters from json directory 
		ofDirectory dir;
		if (!dir.doesDirectoryExist("json")) {
			dir.createDirectory("json");
		} else {
			auto& json = ofLoadJson("json/renderers/" + name + ".json");
			if (!json.is_null()) loadParams(json);
		}

	}

	void Helper::createHelperGui() {
		helperGroup.setup("helper");
		helperGroup.setBackgroundColor(ofFloatColor(0., 0.5));
		helperGroup.setHeaderBackgroundColor(ofFloatColor(0.94, 0.1, 0.2, 0.5));
		helperGroup.setBorderColor(ofFloatColor(0.1, 0.5));
		helperGroup.setFillColor(ofFloatColor(0.3, 0.3, 0.6, 0.5));
		helperGroup.add(debugViewMode.set("debugViewMode", 0, 0, 4));
		saveButton.setup("save");
		saveButton.addListener(this, &Helper::saveParams);
		helperGroup.add(&saveButton);

		loadButton.setup("load");
		loadButton.addListener(this, &Helper::load);
		helperGroup.add(&loadButton);
	}
}