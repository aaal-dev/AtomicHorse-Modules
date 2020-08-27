#include "plugin.hpp"


Plugin* pluginInstance;


void init(Plugin* p) {
	pluginInstance = p;
	p->addModel(modelVCO_1);
	p->addModel(modelEnvelope_1);

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

float EnvelopeKnobParamQuantity::getDisplayValue() {
	float v = getValue();
	if (!module) {
		return v;
	}
	float vv = v * v;
	vv *= .1f;
	vv += displayOffset;
	if (v < 0.0f) {
		return -vv;
	}
	return vv;
}

void EnvelopeKnobParamQuantity::setDisplayValue(float displayValue) {
	if (!module) {
		return;
	}
	displayValue -= displayOffset;
	float v = fabsf(displayValue) / 10.f;
	v = powf(v, 0.5f);
	if (displayValue < 0.0f) {
		setValue(-v);
	}
	else {
		setValue(v);
	}
}

