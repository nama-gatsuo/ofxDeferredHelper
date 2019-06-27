#include "ofxDeferredHelper.h"

using namespace ofxDeferred;

Helper::Helper() {}

void Helper::init(int w, int h) {

	ofLogNotice() << w << "," << h;
	processor.init(w, h);
	createPasses();
}

void Helper::render(std::function<void(float, bool)> drawCall, ofCamera& cam, bool autoDraw) {
	if (shadow && shadow->getEnabled()) {
		shadow->beginShadowMap();
		drawCall(shadow->getLinearScalar(), true);
		drawLights(shadow->getLinearScalar(), true);
		shadow->endShadowMap();
	}

	float lds = 1. / (cam.getFarClip() - cam.getNearClip());
	processor.begin(cam, true);
	drawCall(lds, false);
	drawLights(lds, false);
	processor.end(autoDraw);
}

const ofFbo& Helper::getRenderedImage() const {
	return processor.getFbo();
}

void Helper::save() {
	json.clear();
	ofParameterGroup g;
	g.setName("deferred");
	for (auto& gr : groups) {
		g.add(gr.second.getParameter());
	}
	ofSerialize(json, g);
	ofSaveJson("json/deferred.json", json);
}

void ofxDeferred::Helper::load() {
	json = ofLoadJson("json/deferred.json");
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

void Helper::debugDraw() {
	switch (debugViewMode.get()) {
	case 0: {
		processor.debugDraw();
		if (shadow) shadow->debugDraw();
	}break;
	case 1: {
		if (dof) dof->debugDraw();
	}break;
	case 2: {
		if (bloom) bloom->debugDraw();
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

void Helper::drawLights(float lds, bool isShadow) {
	if (pointLight->getEnabled()) pointLight->drawLights(lds, isShadow);
}


void Helper::createPasses() {

	createAllPasses();
	createGui();

	ofDirectory dir;
	if (!dir.doesDirectoryExist("json")) {
		dir.createDirectory("json");
	} else {
		load();
	}

}

void Helper::createAllPasses() {
	processor.init();
	auto bg = processor.createPass<ofxDeferred::BgPass>();
	bg->begin();
	ofClear(10, 12, 24, 255);
	bg->end();

	auto e = processor.createPass<ofxDeferred::EdgePass>();
	e->setUseReadColor(true);
	e->setEdgeColor(ofFloatColor(0.));
	processor.createPass<ofxDeferred::SsaoPass>();
	shadow = processor.createPass<ofxDeferred::ShadowLightPass>();
	shadow->setPosition(glm::vec3(100., 200., 100.));
	shadow->lookAt(glm::vec3(0.));
	shadow->setFar(800.);
	shadow->setNear(50.);
	shadow->setViewPortSize(256.);
	pointLight = processor.createPass<ofxDeferred::PointLightPass>();
	for (int i = 0; i < 6; i++) {
		pointLight->addLight();
	}
	dof = processor.createPass<ofxDeferred::DofPass>();
	bloom = processor.createPass<ofxDeferred::BloomPass>();
}

void Helper::createGui() {

	float heightSum = 10.;
	float widthSum = 10.;
	for (int i = 0; i < processor.size(); i++) {
		const float dw = 240.;
		const float colorOffset = 280.;
		const std::string& name = processor[i]->getName();

		groups[name].setDefaultWidth(dw);
		groups[name].setBackgroundColor(ofFloatColor(0., 0.5));
		groups[name].setDefaultBackgroundColor(ofFloatColor(0., 0.5));
		groups[name].setHeaderBackgroundColor(ofFloatColor(0.6, 0.3, 0.8, 0.5));
		groups[name].setDefaultHeaderBackgroundColor(ofFloatColor(0.0, 0.5));
		groups[name].setDefaultFillColor(ofFloatColor(0.3, 0.3, 0.6, 0.5));
		groups[name].setBorderColor(ofFloatColor(0.1, 0.5));
		groups[name].setDefaultBorderColor(ofFloatColor(0.1, 0.5));
		groups[name].setup(processor[i]->getParameters());
		groups[name].setName(name);

		float h = groups[name].getHeight();
		float hOffset = 10.;
		if (name == RenderPassRegistry::Bg ||
			name == RenderPassRegistry::Edge ||
			name == RenderPassRegistry::PointLight ||
			name == RenderPassRegistry::ShadowLight) {

			hOffset = colorOffset;
		}

		if (heightSum + h + hOffset > ofGetHeight()) {
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

		// groups[i].loadFromFile("xml/" + name + ".xml");
	}

	helperGroup.setup("helper");
	helperGroup.setPosition(widthSum, heightSum);
	helperGroup.setHeaderBackgroundColor(ofFloatColor(0.94, 0.1, 0.2, 0.5));
	helperGroup.add(debugViewMode.set("debugViewMode", 0, 0, 2));
	saveButton.setup("save");
	saveButton.addListener(this, &Helper::save);
	helperGroup.add(&saveButton);

	loadButton.setup("load");
	loadButton.addListener(this, &Helper::load);
	helperGroup.add(&loadButton);

}
