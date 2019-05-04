#include "ofxDeferredHelper.h"

using namespace ofxDeferred;

Helper::Helper() {}

void Helper::init(int w, int h) {
	processor.init(w, h);
	createPasses();
	createGui();
}

void Helper::render(std::function<void(float, bool)> drawCall, ofCamera& cam, bool autoDraw) {
	shadow->beginShadowMap();
	drawCall(shadow->getLinearScalar(), true);
	pointLight->drawLights(shadow->getLinearScalar(), true);
	shadow->endShadowMap();

	float lds = 1. / (cam.getFarClip() - cam.getNearClip());
	processor.begin(cam, true);
	drawCall(lds, false);
	pointLight->drawLights(lds, false);
	processor.end(autoDraw);
}

const ofFbo& Helper::getRenderedImage() const {
	return processor.getFbo();
}

void Helper::save() {
	for (auto& g : groups) {
		g.saveToFile("xml/" + g.getName() + ".xml");
	}
}

void Helper::drawGbuffer() {
	processor.debugDraw();
	shadow->debugDraw();
}

void Helper::drawGui() {
	ofPushStyle();
	ofEnableAlphaBlending();
	for (auto& g : groups) {
		g.draw();
	}
	ofPopStyle();
}

void Helper::createPasses() {
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
	shadow->setPosition(glm::vec3(100., 300., 100.));
	shadow->lookAt(glm::vec3(0.));
	shadow->setFar(800.);
	shadow->setNear(50.);
	shadow->setViewPortSize(256.);
	pointLight = processor.createPass<ofxDeferred::PointLightPass>();
	for (int i = 0; i < 6; i++) {
		pointLight->addLight();
	}
	processor.createPass<ofxDeferred::DofPass>();
	processor.createPass<ofxDeferred::BloomPass>();
}
void Helper::createGui() {
	groups.assign(processor.size(), ofxGuiGroup());
	float heightSum = 10.;
	float widthSum = 10.;
	for (int i = 0; i < processor.size(); i++) {
		const float dw = 240.;
		const float colorOffset = 280.;
		const std::string& name = processor[i]->getName();

		groups[i].setDefaultWidth(dw);
		groups[i].setBackgroundColor(ofFloatColor(0., 0.5));
		groups[i].setDefaultBackgroundColor(ofFloatColor(0., 0.5));
		groups[i].setHeaderBackgroundColor(ofFloatColor(0.6, 0.3, 0.8, 0.5));
		groups[i].setDefaultHeaderBackgroundColor(ofFloatColor(0.0, 0.5));
		groups[i].setDefaultFillColor(ofFloatColor(0.3, 0.3, 0.6, 0.5));
		groups[i].setBorderColor(ofFloatColor(0.1, 0.5));
		groups[i].setDefaultBorderColor(ofFloatColor(0.1, 0.5));
		groups[i].setup(processor[i]->getParameters(), "xml/" + name + ".xml");
		groups[i].setName(name);

		float h = groups[i].getHeight();
		float hOffset = 10.;
		if (name == "BgPass" || name == "EdgePass" || name == "PointLightPass") {
			hOffset = colorOffset;
		} else if (name == "ShadowLightPass") {
			hOffset = colorOffset;
		}
		
		if (heightSum + h + hOffset > ofGetHeight()) {
			// line break
			widthSum += dw + 10.;
			groups[i].setPosition(widthSum, 10.);
			heightSum = h + 10.;
		} else {
			groups[i].setPosition(widthSum, heightSum);
			heightSum += h;
		}

		if (name == "PointLightPass") {
			groups[i].minimizeAll();
		}
		
		heightSum += hOffset;
		
		groups[i].loadFromFile("xml/" + name + ".xml");
	}
}