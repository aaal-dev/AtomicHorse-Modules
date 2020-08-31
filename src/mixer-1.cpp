#include "mixer-1.hpp"


MixerMainLevelKnob::MixerMainLevelKnob() {
	setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/Mixer_MaimLevelKnob.svg")));
	shadow->box.pos = Vec(0.0, 2.5);
	minAngle = -0.5 * M_PI;
	maxAngle = 0.8 * M_PI;
}

template <typename TLightBase>
LEDLightSliderFixed<TLightBase>::LEDLightSliderFixed() {
	this->setHandleSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/LEDSliderHandle.svg")));
}

Mixer_1::Mixer_1() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configParam(SLIDER_LEVEL1_PARAM, 0.f, 1.3f, 1.f, "Level of signal 1", "%", 0.f, 100.f);
	configParam(SLIDER_LEVEL2_PARAM, 0.f, 1.3f, 1.f, "Level of signal 2", "%", 0.f, 100.f);
	configParam(SLIDER_LEVEL3_PARAM, 0.f, 1.3f, 1.f, "Level of signal 3", "%", 0.f, 100.f);
	configParam(SLIDER_LEVEL4_PARAM, 0.f, 1.3f, 1.f, "Level of signal 4", "%", 0.f, 100.f);

	configParam(KNOB_PAN1_PARAM, -1.f, 1.f, 0.f, "Panning of signal 1", "%", 0.f, 100.f);
	configParam(KNOB_PAN2_PARAM, -1.f, 1.f, 0.f, "Panning of signal 2", "%", 0.f, 100.f);
	configParam(KNOB_PAN3_PARAM, -1.f, 1.f, 0.f, "Panning of signal 3", "%", 0.f, 100.f);
	configParam(KNOB_PAN4_PARAM, -1.f, 1.f, 0.f, "Panning of signal 4", "%", 0.f, 100.f);

	configParam(KNOB_LEVELMAIN_PARAM, 0.f, 1.3f, 1.f, "Level of mixed signal", "%", 0.f, 100.f);
}

Mixer_1Widget::Mixer_1Widget(Mixer_1* module) {
	setModule(module);
	setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/mixer-1.svg")));

	addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	addParam(createLightParamCentered<LEDLightSliderFixed<GreenLight>>(mm2px(Vec(6.820, 46.464)), module, Mixer_1::SLIDER_LEVEL1_PARAM, Mixer_1::LED_LEVEL1_LIGHT));
	addParam(createLightParamCentered<LEDLightSliderFixed<GreenLight>>(mm2px(Vec(15.82, 46.464)), module, Mixer_1::SLIDER_LEVEL2_PARAM, Mixer_1::LED_LEVEL2_LIGHT));
	addParam(createLightParamCentered<LEDLightSliderFixed<GreenLight>>(mm2px(Vec(24.82, 46.464)), module, Mixer_1::SLIDER_LEVEL3_PARAM, Mixer_1::LED_LEVEL3_LIGHT));
	addParam(createLightParamCentered<LEDLightSliderFixed<GreenLight>>(mm2px(Vec(33.82, 46.464)), module, Mixer_1::SLIDER_LEVEL4_PARAM, Mixer_1::LED_LEVEL4_LIGHT));

	addParam(createParamCentered<AHMRoundKnobWhiteTiny>(mm2px(Vec(6.820, 85.312)), module, Mixer_1::KNOB_PAN1_PARAM));
	addParam(createParamCentered<AHMRoundKnobWhiteTiny>(mm2px(Vec(15.82, 85.312)), module, Mixer_1::KNOB_PAN2_PARAM));
	addParam(createParamCentered<AHMRoundKnobWhiteTiny>(mm2px(Vec(24.82, 85.312)), module, Mixer_1::KNOB_PAN3_PARAM));
	addParam(createParamCentered<AHMRoundKnobWhiteTiny>(mm2px(Vec(33.82, 85.312)), module, Mixer_1::KNOB_PAN4_PARAM));

	addParam(createParamCentered<AHMRoundKnobWhiteTiny>(mm2px(Vec(6.820, 106.566)), module, Mixer_1::KNOB_CVMAIN_PARAM));
	addParam(createParamCentered<MixerMainLevelKnob>(mm2px(Vec(20.32, 111.093)), module, Mixer_1::KNOB_LEVELMAIN_PARAM));

	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.820, 13.0)), module, Mixer_1::JACK_IN1_L_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.820, 22.0)), module, Mixer_1::JACK_IN1_R_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.82, 13.0)), module, Mixer_1::JACK_IN2_L_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.82, 22.0)), module, Mixer_1::JACK_IN2_R_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24.82, 13.0)), module, Mixer_1::JACK_IN3_L_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24.82, 22.0)), module, Mixer_1::JACK_IN3_R_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(33.82, 13.0)), module, Mixer_1::JACK_IN4_L_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(33.82, 22.0)), module, Mixer_1::JACK_IN4_R_INPUT));

	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.820, 70.31)), module, Mixer_1::JACK_CV1_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.82, 70.31)), module, Mixer_1::JACK_CV2_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24.82, 70.31)), module, Mixer_1::JACK_CV3_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(33.82, 70.31)), module, Mixer_1::JACK_CV4_INPUT));

	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.820, 93.63)), module, Mixer_1::JACK_PAN1_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.82, 93.63)), module, Mixer_1::JACK_PAN2_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24.82, 93.63)), module, Mixer_1::JACK_PAN3_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(33.82, 93.63)), module, Mixer_1::JACK_PAN4_INPUT));

	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.82, 115.566)), module, Mixer_1::JACK_CVMAIN_INPUT));

	addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(33.82, 106.566)), module, Mixer_1::JACK_OUTMAIN_L_OUTPUT));
	addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(33.82, 115.566)), module, Mixer_1::JACK_OUTMAIN_R_OUTPUT));
}

Model* modelMixer_1 = createModel<Mixer_1, Mixer_1Widget>("Mixer-1");
