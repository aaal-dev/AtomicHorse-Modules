#include "mixer-2m-1s.hpp"

Model* model_Mixer2m1s = createModel<Mixer2m1s, Mixer2m1s_Widget>("Mixer-2m-1s");

Mixer2m1s::Mixer2m1s() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configParam(FADER_LEVEL_L_PARAM, 0.f, M_SQRT2, 1.f, "Level of left channel");
	configParam(FADER_LEVEL_R_PARAM, 0.f, M_SQRT2, 1.f, "Level of right channel");
	configParam(KNOB_PAN_L_PARAM, -1.f, 1.f, 0.f, "Panning of left channel");
	configParam(KNOB_PAN_R_PARAM, -1.f, 1.f, 0.f, "Panning of right channel");
	configParam(KNOB_MAINLEVEL_PARAM, 0.f, M_SQRT2, 1.f, "Level of mixed signal");
}

Mixer2m1s_Widget::Mixer2m1s_Widget(Mixer2m1s* module) {
	setModule(module);
	setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/mixer-2m-1s.svg")));

	addChild(createWidget<ScrewSilver>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
	addChild(createWidget<ScrewSilver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec( 5.66, 13.0)), module, Mixer2m1s::JACK_MAIN_L_OUTPUT));
	addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(14.66, 13.0)), module, Mixer2m1s::JACK_MAIN_R_OUTPUT));

	addParam(createParamCentered<Mixer2m1s::MixerMainLevelKnob>(mm2px(Vec(10.16, 28.33)), module, Mixer2m1s::KNOB_MAINLEVEL_PARAM));

	addParam(createParamCentered<AHMRoundKnobWhiteTiny>(mm2px(Vec(5.66, 40.25)), module, Mixer2m1s::KNOB_MAINCV_PARAM));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.66, 40.25)), module, Mixer2m1s::JACK_MAINCV_INPUT));

	addParam(createParamCentered<Mixer2m1s::MixerLevel>(mm2px(Vec( 5.66, 63.464)), module, Mixer2m1s::FADER_LEVEL_L_PARAM));
	addParam(createParamCentered<Mixer2m1s::MixerLevel>(mm2px(Vec(14.66, 63.464)), module, Mixer2m1s::FADER_LEVEL_R_PARAM));
	addParam(createParamCentered<AHMRoundKnobWhiteTiny>(mm2px(Vec( 5.66, 89.138)), module, Mixer2m1s::KNOB_PAN_L_PARAM));
	addParam(createParamCentered<AHMRoundKnobWhiteTiny>(mm2px(Vec(14.66, 89.138)), module, Mixer2m1s::KNOB_PAN_R_PARAM));

	addInput(createInputCentered<PJ301MPort>(mm2px(Vec( 5.66,  97.63 )), module, Mixer2m1s::JACK_PAN_L_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.66,  97.63 )), module, Mixer2m1s::JACK_PAN_R_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec( 5.66, 106.566)), module, Mixer2m1s::JACK_CV_L_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.66, 106.566)), module, Mixer2m1s::JACK_CV_R_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec( 5.66, 115.061)), module, Mixer2m1s::JACK_IN_L_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.66, 115.061)), module, Mixer2m1s::JACK_IN_R_INPUT));
}

void Mixer2m1s::process(const ProcessArgs& args) {
	int voices_needed = 1;

	float fader_level_l = std::pow(params[FADER_LEVEL_L_PARAM].getValue(), 2.f);
	float fader_level_r = std::pow(params[FADER_LEVEL_R_PARAM].getValue(), 2.f);
	float pan_l = params[KNOB_PAN_L_PARAM].getValue();
	float pan_r = params[KNOB_PAN_R_PARAM].getValue();
	float pan_level_ll = 1.f;
	float pan_level_lr = 1.f;
	float pan_level_rl = 1.f;
	float pan_level_rr = 1.f;

	float input_level_l[16] = {0.f};
	float input_level_r[16] = {0.f};
	float output_level_ll[16] = {0.f};
	float output_level_lr[16] = {0.f};
	float output_level_rl[16] = {0.f};
	float output_level_rr[16] = {0.f};

	float main_level_param = params[KNOB_MAINLEVEL_PARAM].getValue();
	float main_cv_param = params[KNOB_MAINCV_PARAM].getValue();

	// Left channel
	if (inputs[JACK_IN_L_INPUT].isConnected()) {
		voices_needed = std::max(voices_needed, inputs[JACK_IN_L_INPUT].getChannels());
		inputs[JACK_IN_L_INPUT].readVoltages(input_level_l);
		if (pan_l < 0.f) {
			pan_l = 0.f - pan_l;
			pan_level_ll = 1.f;
			pan_level_lr = 1.f - pan_l;
		} else if (pan_l > 0.f) {
			pan_level_ll = 1.f - pan_l;
			pan_level_lr = 1.f;
		}
	}

	// Right channel
	if (inputs[JACK_IN_R_INPUT].isConnected()) {
		voices_needed = std::max(voices_needed, inputs[JACK_IN_R_INPUT].getChannels());
		inputs[JACK_IN_R_INPUT].readVoltages(input_level_r);
		if (pan_r < 0.f) {
			pan_r = 0.f - pan_r;
			pan_level_rl = 1.f;
			pan_level_rr = 1.f - pan_r;
			//pan_level_lr -= pan_r;
		} else if (pan_r > 0.f) {
			pan_level_rl = 1.f - pan_r;
			pan_level_rr = 1.f;
			//pan_level_ll -= pan_r;
		}
	}

	for (int voice = 0; voice < voices_needed; voice++) {

		if (inputs[JACK_MAINCV_INPUT].isConnected()) {
			main_cv_param *= clamp(inputs[JACK_MAINCV_INPUT].getPolyVoltage(voice) / 10.f, 0.f, 1.f);
			main_level_param *= main_cv_param;
		}

		// Left channel
		if (outputs[JACK_MAIN_L_OUTPUT].isConnected()) {
			input_level_l[voice] *= fader_level_l;
			if (inputs[JACK_CV_L_INPUT].isConnected())
				input_level_l[voice] *= clamp(inputs[JACK_CV_L_INPUT].getPolyVoltage(voice) / 10.f, 0.f, 1.f);
		}

		// Right channel
		if (outputs[JACK_MAIN_R_OUTPUT].isConnected()) {
			input_level_r[voice] *= fader_level_r;
			if (inputs[JACK_CV_R_INPUT].isConnected())
				input_level_r[voice] *= clamp(inputs[JACK_CV_R_INPUT].getPolyVoltage(voice) / 10.f, 0.f, 1.f);
		}

		output_level_ll[voice] = input_level_l[voice] * pan_level_ll / (1.f + pan_level_rl * fader_level_r);
		output_level_lr[voice] = input_level_l[voice] * pan_level_lr / (1.f + pan_level_rr * fader_level_r);

		output_level_rl[voice] = input_level_r[voice] * pan_level_rl / (1.f + pan_level_ll * fader_level_l);
		output_level_rr[voice] = input_level_r[voice] * pan_level_rr / (1.f + pan_level_lr * fader_level_l);

		main_l_value[voice] = output_level_ll[voice] + output_level_rl[voice];
		main_l_value[voice] *= main_level_param;

		main_r_value[voice] = output_level_lr[voice] + output_level_rr[voice];
		main_r_value[voice] *= main_level_param;
	}
	outputs[JACK_MAIN_L_OUTPUT].setChannels(voices_needed);
	outputs[JACK_MAIN_R_OUTPUT].setChannels(voices_needed);
	outputs[JACK_MAIN_L_OUTPUT].writeVoltages(main_l_value);
	outputs[JACK_MAIN_R_OUTPUT].writeVoltages(main_r_value);

}



Mixer2m1s::MixerMainLevelKnob::MixerMainLevelKnob() {
	setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/Mixer_MainLevelKnob.svg")));
	shadow->box.pos = Vec(0.0, 2.5);
	minAngle = -0.5 * M_PI;
	maxAngle = 0.941 * M_PI;
}

Mixer2m1s::MixerLevel::MixerLevel() {
	setBackgroundSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/Mixer_SliderPot.svg")));
	setHandleSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/Mixer_SliderHandle.svg")));
	maxHandlePos = mm2px(Vec(1.25, 0));
	minHandlePos = mm2px(Vec(1.25, 30));
	//background->box.pos = margin;
	//box.size = background->box.size.plus(margin.mult(2));
}
