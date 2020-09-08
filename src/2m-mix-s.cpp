#include "2m-mix-s.hpp"

Model* modelM2_MIX_S = createModel<M2_MIX_S, M2_MIX_S_Widget>("2m-mix-s");

M2_MIX_S::M2_MIX_S() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	for (int i = 0; i < TRACKS_NUMBER; i++) {
		configParam(FADER_LEVEL_PARAM + i, 0.f, M_SQRT2, 1.f, "Level of signal");
		configParam(KNOB_PAN_PARAM + i, -1.f, 1.f, 0.f, "Panning of signal");
	}
	configParam(KNOB_MAINLEVEL_PARAM, 0.f, M_SQRT2, 1.f, "Level of mixed signal");
}

M2_MIX_S_Widget::M2_MIX_S_Widget(M2_MIX_S* module) {
	setModule(module);
	setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/2m-mix-s.svg")));

	addChild(createWidget<ScrewSilver>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
	addChild(createWidget<ScrewSilver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec( 5.66, 13.0)), module, M2_MIX_S::JACK_MAIN_L_OUTPUT));
	addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(14.66, 13.0)), module, M2_MIX_S::JACK_MAIN_R_OUTPUT));

	addParam(createParamCentered<M2_MIX_S::MixerMainLevelKnob>(mm2px(Vec(10.16, 28.33)), module, M2_MIX_S::KNOB_MAINLEVEL_PARAM));

	addParam(createParamCentered<AHMRoundKnobWhiteTiny>(mm2px(Vec(5.66, 40.25)), module, M2_MIX_S::KNOB_MAINCV_PARAM));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.66, 40.25)), module, M2_MIX_S::JACK_MAINCV_INPUT));

	for (int i = 0; i < TRACKS_NUMBER; i++) {
		addParam(createParamCentered<M2_MIX_S::MixerLevel>(mm2px(Vec(5.66 + 9 * i, 63.464)), module, M2_MIX_S::FADER_LEVEL_PARAM + i));
		addParam(createParamCentered<AHMRoundKnobWhiteTiny>(mm2px(Vec(5.66 + 9 * i, 89.138)), module, M2_MIX_S::KNOB_PAN_PARAM + i));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.66 + 9 * i,  97.63)), module, M2_MIX_S::JACK_PAN_INPUT + i));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.66 + 9 * i, 106.566)), module, M2_MIX_S::JACK_CV_INPUT + i));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.66 + 9 * i, 115.061)), module, M2_MIX_S::JACK_IN_INPUT + i));
	}
}

void M2_MIX_S::process(const ProcessArgs& args) {
	int voices_needed = 1;

	// Tracks
	for (int i = 0; i < TRACKS_NUMBER; i++) {
		float fader_level = params[FADER_LEVEL_PARAM + i].getValue();
		float pan_level = params[KNOB_PAN_PARAM + i].getValue();
		float input_level[16] = {};

		if (inputs[JACK_IN_INPUT + i].isConnected()) {
			int voices = inputs[JACK_IN_INPUT + i].getChannels();
			voices_needed = std::max(voices_needed, voices);
			inputs[JACK_IN_INPUT + i].readVoltages(input_level);
			for (int voice = 0; voice < voices; voice++) {
				input_level[voice] *= std::pow(fader_level, 2.f);
				if (inputs[JACK_CV_INPUT + i].isConnected())
					input_level[voice] *= clamp(inputs[JACK_CV_INPUT + i].getPolyVoltage(voice) / 10.f, 0.f, 1.f);
				float input_level_l = input_level[voice]/2;
				float input_level_r = input_level[voice]/2;
				if (pan_level < 0.f)
					input_level_r = input_level_r * (0.f - pan_level);
				if (pan_level > 0.f)
					input_level_l = input_level_l * pan_level;

				main_l_value[voice] += input_level_l;
				main_r_value[voice] += input_level_r;
			}
		}
	}

	// Main mix
	float main_level_param = params[KNOB_MAINLEVEL_PARAM].getValue();
	float main_cv_param = params[KNOB_MAINCV_PARAM].getValue();


	for (int voice = 0; voice < voices_needed; voice++) {

		// Left channel
		if (outputs[JACK_MAIN_L_OUTPUT].isConnected()) {
			if (inputs[JACK_MAINCV_INPUT].isConnected()) {
				float mainCV = inputs[JACK_MAINCV_INPUT].getPolyVoltage(voice) * main_cv_param;
				main_l_value[voice] += mainCV;
			}
			main_l_value[voice] *= main_level_param;
		}


		// Right channel
		if (outputs[JACK_MAIN_R_OUTPUT].isConnected()) {
			if (inputs[JACK_MAINCV_INPUT].isConnected()) {
				float mainCV = inputs[JACK_MAINCV_INPUT].getPolyVoltage(voice) * main_cv_param;
				main_r_value[voice] += mainCV;
			}
			main_r_value[voice] *= main_level_param;
		}
	}
	outputs[JACK_MAIN_L_OUTPUT].setChannels(voices_needed);
	outputs[JACK_MAIN_R_OUTPUT].setChannels(voices_needed);
	outputs[JACK_MAIN_L_OUTPUT].writeVoltages(main_l_value);
	outputs[JACK_MAIN_R_OUTPUT].writeVoltages(main_r_value);

}



M2_MIX_S::MixerMainLevelKnob::MixerMainLevelKnob() {
	setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/Mixer_MaimLevelKnob.svg")));
	shadow->box.pos = Vec(0.0, 2.5);
	minAngle = -0.5 * M_PI;
	maxAngle = 0.941 * M_PI;
}

M2_MIX_S::MixerLevel::MixerLevel() {
	setBackgroundSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/Mixer_SliderPot.svg")));
	setHandleSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/Mixer_SliderHandle.svg")));
	maxHandlePos = mm2px(Vec(1.25, 0));
	minHandlePos = mm2px(Vec(1.25, 30));
	//background->box.pos = margin;
	//box.size = background->box.size.plus(margin.mult(2));
}
