#include "plugin.hpp"


Plugin* pluginInstance;


void init(Plugin* p) {
	pluginInstance = p;
	p->addModel(modelVCO_1);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables 
	// when your module is created to reduce startup times of Rack.
}

AHMRoundKnobWhiteHuge::AHMRoundKnobWhiteHuge() {
	setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/AHMRoundKnobWhiteHuge.svg")));
  shadow->box.pos = Vec(0.0, 2.5);
  minAngle = -0.8 * M_PI;
  maxAngle = 0.8 * M_PI;
}

AHMRoundKnob2WhiteHuge::AHMRoundKnob2WhiteHuge() {
	setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/AHMRoundKnob2WhiteHuge.svg")));
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

AHMRotaryTumblerWhiteHugeSnap::AHMRotaryTumblerWhiteHugeSnap() {
	setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/AHMRotaryTumblerWhiteHuge.svg")));
	minAngle = -0.3 * M_PI;
    maxAngle = 0.3 * M_PI;
	snap = true;
	shadow->opacity = 0.f;
}

float VCOFreqKnobParamQuantity::offset() {
	return 0.0f;
}

float VCOFreqKnobParamQuantity::getDisplayValue() {
	float v = getValue();
	if (!module) {
		return v;
	}
	v += offset();
	v = powf(2.0f, v);
	v *= dsp::FREQ_C4;
	return v;
}

void VCOFreqKnobParamQuantity::setDisplayValue(float v) {
	if (!module) {
		return;
	}
	v /= dsp::FREQ_C4;
	v = log2f(v);
	v -= offset();
	setValue(v);
}
