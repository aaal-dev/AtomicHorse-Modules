#include "plugin.hpp"


Plugin* pluginInstance;


void init(Plugin* p) {
	pluginInstance = p;
	p->addModel(model_VCO_1);
	p->addModel(model_Envelope_1);
	p->addModel(model_ADSR);
	p->addModel(model_LFO_1);
	p->addModel(model_Mixer_1);
	p->addModel(model_Mixer2m1s);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables 
	// when your module is created to reduce startup times of Rack.
}

AHMRoundKnobWhite::AHMRoundKnobWhite() {
	setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/AHMRoundKnobWhite.svg")));
	shadow->box.pos = Vec(0.0, 2.5);
	minAngle = -0.8 * M_PI;
	maxAngle = 0.8 * M_PI;
}

AHMRoundKnobWhiteHuge::AHMRoundKnobWhiteHuge() {
	setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/AHMRoundKnobWhiteHuge.svg")));
	shadow->box.pos = Vec(0.0, 2.5);
	minAngle = -0.8 * M_PI;
	maxAngle = 0.8 * M_PI;
}

AHMRoundKnobWhiteLarge::AHMRoundKnobWhiteLarge() {
	setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/AHMRoundKnobWhiteLarge.svg")));
	shadow->box.pos = Vec(0.0, 2.5);
	minAngle = -0.66667 * M_PI;
	maxAngle = 0.66667 * M_PI;
}

AHMRoundKnobWhiteTiny::AHMRoundKnobWhiteTiny() {
	setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/AHMRoundKnobWhiteTiny.svg")));
	shadow->box.pos = Vec(0.0, 2.5);
	minAngle = -0.8 * M_PI;
    maxAngle = 0.8 * M_PI;
}

AHMRoundKnob2WhiteTiny::AHMRoundKnob2WhiteTiny() {
	setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/AHMRoundKnob2WhiteTiny.svg")));
	shadow->box.pos = Vec(0.0, 2.5);
	minAngle = -0.8 * M_PI;
	maxAngle = 0.8 * M_PI;
}

AHMRoundKnob3WhiteTiny::AHMRoundKnob3WhiteTiny() {
	setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/AHMRoundKnob3WhiteTiny.svg")));
	shadow->box.pos = Vec(0.0, 2.5);
	minAngle = -0.8 * M_PI;
	maxAngle = 0.8 * M_PI;
}

AHMRoundKnob4WhiteTiny::AHMRoundKnob4WhiteTiny() {
	setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/AHMRoundKnob4WhiteTiny.svg")));
	shadow->box.pos = Vec(0.0, 2.5);
	minAngle = -0.5 * M_PI;
	maxAngle = 0.5 * M_PI;
}

AHMCKSS::AHMCKSS() {
	addFrame(APP->window->loadSvg(asset::system("res/components/AHMCKSS_0.svg")));
	addFrame(APP->window->loadSvg(asset::system("res/components/AHMCKSS_1.svg")));
}




