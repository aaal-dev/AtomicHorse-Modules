#include "mixer-4m-1m.hpp"

Model* model_Mixer4m1m = createModel<Mixer4m1m, Mixer4m1m_Widget>("Mixer-4m-1m");

Mixer4m1m::Mixer4m1m() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	for (int track = 0; track < TRACKS_NUMBER; track++) {
		configParam(FADER_LEVEL_PARAM + track, 0.f, M_SQRT2, 1.f, "Level of signal " + std::to_string(track + 1), "%", 0.f, 100.f);
	}
	configParam(KNOB_MAINCV_PARAM, 0.f, 1.f, 0.f, "Main level modulation depth", "%", 0.f, 100.f);
	configParam(KNOB_MAINLEVEL_PARAM, 0.f, M_SQRT2, 1.f, "Level of mixed signal", "%", 0.f, 100.f);
}

Mixer4m1m_Widget::Mixer4m1m_Widget(Mixer4m1m* module) {
	setModule(module);
	setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/mixer-4m-1m.svg")));

	addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	addParam(createParamCentered<AHMRoundKnobWhiteTiny>(mm2px(Vec(6.820, 13.472)), module, Mixer4m1m::KNOB_MAINCV_PARAM));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.82, 22.498)), module, Mixer4m1m::JACK_MAINCV_INPUT));

	addParam(createParamCentered<Mixer4m1m::MixerMainLevelKnob>(mm2px(Vec(20.32, 18.0)), module, Mixer4m1m::KNOB_MAINLEVEL_PARAM));
	addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(33.82, 18.0)), module, Mixer4m1m::JACK_MAIN_OUTPUT));

	for (int track = 0; track < TRACKS_NUMBER; track++) {
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(6.82 + 9 * track,  46.464)), module, Mixer4m1m::JACK_OUT_OUTPUT + track));
		addParam(createParamCentered<Mixer4m1m::MixerLevel>(mm2px(Vec(6.82 + 9 * track, 76.630)), module, Mixer4m1m::FADER_LEVEL_PARAM + track));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.82 + 9 * track, 106.566)), module, Mixer4m1m::JACK_CV_INPUT + track));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.82 + 9 * track, 115.061)), module, Mixer4m1m::JACK_IN_INPUT + track));
	}
}

void Mixer4m1m::process(const ProcessArgs& args) {
	int voices_needed = 1;

	// Tracks
	float track_value[TRACKS_NUMBER][16] = {0.f};
	float fader_level[TRACKS_NUMBER] = {0.f};

	for (int track = 0; track < TRACKS_NUMBER; track++) {
		fader_level[track] = std::pow(params[FADER_LEVEL_PARAM + track].getValue(), 2.f);

		if (inputs[JACK_IN_INPUT + track].isConnected()) {
			voices_needed = std::max(voices_needed, inputs[JACK_IN_INPUT + track].getChannels());
			inputs[JACK_IN_INPUT + track].readVoltages(track_value[track]);
			outputs[JACK_OUT_OUTPUT + track].setChannels(voices_needed);
			for (int voice = 0; voice < voices_needed; voice++) {
				track_value[track][voice] *= fader_level[track];
				if (inputs[JACK_CV_INPUT + track].isConnected())
					track_value[track][voice] *= clamp(inputs[JACK_CV_INPUT + track].getPolyVoltage(voice) / 10.f, 0.f, 1.f);
				outputs[JACK_OUT_OUTPUT + track].setVoltage(track_value[track][voice], voice);
			}
		}
	}


	// Main mix
	float main_value[16] = {0.f};
	float main_level_param = params[KNOB_MAINLEVEL_PARAM].getValue();
	float main_cv_param = params[KNOB_MAINCV_PARAM].getValue();

	for (int voice = 0; voice < voices_needed; voice++) {
		if (outputs[JACK_MAIN_OUTPUT].isConnected()) {
			for (int track = 0; track < TRACKS_NUMBER; track++) {
				main_value[voice] += track_value[track][voice] / compensateVolume(track, fader_level);
				if (inputs[JACK_MAINCV_INPUT].isConnected()) {
					float main_cv = clamp(inputs[JACK_MAINCV_INPUT].getPolyVoltage(voice) / 10.f, 0.f, 1.f);
					main_value[voice] *= main_cv * main_cv_param;
				}
			}
			main_value[voice] *= main_level_param;
		}
	}

	outputs[JACK_MAIN_OUTPUT].setChannels(voices_needed);
	outputs[JACK_MAIN_OUTPUT].writeVoltages(main_value);
}


// External functions

float Mixer4m1m::compensateVolume(int current_track_number, float fader_level[]) {
	float value = 0.f;
	for (int track = 1; track < TRACKS_NUMBER; track++) {
		int track_number = current_track_number - track;
		track_number = track_number < 0 ? TRACKS_NUMBER + track_number : track_number;
		value += fader_level[track_number];
	}
	return 1 + value;
}



// Additional components

Mixer4m1m::MixerMainLevelKnob::MixerMainLevelKnob() {
	setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/Mixer_MainLevelKnob.svg")));
	shadow->box.pos = Vec(0.0, 2.5);
	minAngle = -0.5 * M_PI;
	maxAngle = 0.941 * M_PI;
}

Mixer4m1m::MixerLevel::MixerLevel() {
	setBackgroundSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/Mixer_SliderPot.svg")));
	setHandleSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/Mixer_SliderHandle.svg")));
	maxHandlePos = mm2px(Vec(1.25, 0));
	minHandlePos = mm2px(Vec(1.25, 30));
	//background->box.pos = margin;
	//box.size = background->box.size.plus(margin.mult(2));
}
