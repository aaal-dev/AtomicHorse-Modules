#include "mixer-1.hpp"


Mixer_1::Mixer_1() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	for (int i = 0; i < TRACKS_NUMBER; i++) {
		configParam(SLIDER_LEVEL_PARAM + i, 0.f, 1.3f, 1.f, "Level of signal " + std::to_string(i + 1), "%", 0.f, 100.f);
		configParam(KNOB_PAN_PARAM + i, -1.f, 1.f, 0.f, "Panning of signal " + std::to_string(i + 1), "%", 0.f, 100.f);
	}
	configParam(KNOB_MAINLEVEL_PARAM, 0.f, 1.f, 0.5f, "Level of mixed signal", "%", 0.f, 100.f);
}

void Mixer_1::process(const ProcessArgs& args) {
	int maxVoices = 1;

	// Tracks
	for (int i = 0; i < TRACKS_NUMBER; i++) {


		// Left channel
		if (inputs[JACK_IN_L_INPUT + i].isConnected()) {
			int voices = inputs[JACK_IN_L_INPUT + i].getChannels();
			maxVoices = std::max(maxVoices, voices);
			for (int voice = 0; voice < voices; voice++) {
				float levelValue = inputs[JACK_IN_L_INPUT + i].getPolyVoltage(voice);
				levelValue *= params[SLIDER_LEVEL_PARAM + i].getValue();
				if (inputs[JACK_CV_INPUT + i].isConnected())
					levelValue *= clamp(inputs[JACK_CV_INPUT + i].getPolyVoltage(voice) / 10.f, 0.f, 1.f);
				main_l_value[voice] += levelValue;
			}
		}

		// Right channel
		if (inputs[JACK_IN_R_INPUT + i].isConnected()) {
			int voices = inputs[JACK_IN_R_INPUT + i].getChannels();
			maxVoices = std::max(maxVoices, voices);
			for (int voice = 0; voice < voices; voice++) {
				float levelValue = inputs[JACK_IN_R_INPUT + i].getPolyVoltage(voice);
				levelValue *= params[SLIDER_LEVEL_PARAM + i].getValue();
				if (inputs[JACK_CV_INPUT + i].isConnected())
					levelValue *= clamp(inputs[JACK_CV_INPUT + i].getPolyVoltage(voice) / 10.f, 0.f, 1.f);
				main_r_value[voice] += levelValue;
			}
		}
	}

	// Main mix
	float main_level_param = params[KNOB_MAINLEVEL_PARAM].getValue();
	float main_cv_param = params[KNOB_MAINCV_PARAM].getValue();
	outputs[JACK_MAIN_L_OUTPUT].setChannels(maxVoices);
	outputs[JACK_MAIN_R_OUTPUT].setChannels(maxVoices);

	for (int voice = 0; voice < maxVoices; voice++) {

		// Left channel
		if (outputs[JACK_MAIN_L_OUTPUT].isConnected()) {
			if (inputs[JACK_MAINCV_INPUT].isConnected()) {
				float mainCV = inputs[JACK_MAINCV_INPUT].getPolyVoltage(voice) * main_cv_param;
				main_l_value[voice] += mainCV;
			}
			main_l_value[voice] *= main_level_param;
			outputs[JACK_MAIN_L_OUTPUT].setVoltage(main_l_value[voice], voice);
		}


		// Right channel
		if (outputs[JACK_MAIN_R_OUTPUT].isConnected()) {
			if (inputs[JACK_MAINCV_INPUT].isConnected()) {
				float mainCV = inputs[JACK_MAINCV_INPUT].getPolyVoltage(voice) * main_cv_param;
				main_r_value[voice] += mainCV;
			}
			main_r_value[voice] *= main_level_param;
			outputs[JACK_MAIN_R_OUTPUT].setVoltage(main_r_value[voice], voice);
		}
	}

}

Mixer_1Widget::Mixer_1Widget(Mixer_1* module) {
	setModule(module);
	setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/mixer-1.svg")));

	addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	//MixerVuMeter* vuMeter1L = createWidget<MixerVuMeter>(mm2px(Vec(3.0, 29.464)));
	//vuMeter1L->module = module;
	//addChild(vuMeter1L);

	for (int i = 0; i < TRACKS_NUMBER; i++) {
		addParam(createParamCentered<MixerLevel>(mm2px(Vec(6.82 + 9 * i, 46.464)), module, Mixer_1::SLIDER_LEVEL_PARAM + i));
		addParam(createParamCentered<AHMRoundKnobWhiteTiny>(mm2px(Vec(6.82 + 9 * i, 85.312)), module, Mixer_1::KNOB_PAN_PARAM + i));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.82 + 9 * i, 13.0)), module, Mixer_1::JACK_IN_L_INPUT + i));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.82 + 9 * i, 22.0)), module, Mixer_1::JACK_IN_R_INPUT + i));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.82 + 9 * i, 70.31)), module, Mixer_1::JACK_CV_INPUT + i));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.82 + 9 * i, 93.63)), module, Mixer_1::JACK_PAN_INPUT + i));
	}

	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.82, 115.566)), module, Mixer_1::JACK_MAINCV_INPUT));
	addParam(createParamCentered<AHMRoundKnobWhiteTiny>(mm2px(Vec(6.820, 106.566)), module, Mixer_1::KNOB_MAINCV_PARAM));
	addParam(createParamCentered<MixerMainLevelKnob>(mm2px(Vec(20.32, 111.093)), module, Mixer_1::KNOB_MAINLEVEL_PARAM));
	addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(33.82, 106.566)), module, Mixer_1::JACK_MAIN_L_OUTPUT));
	addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(33.82, 115.566)), module, Mixer_1::JACK_MAIN_R_OUTPUT));
}

Model* modelMixer_1 = createModel<Mixer_1, Mixer_1Widget>("Mixer-1");

MixerMainLevelKnob::MixerMainLevelKnob() {
	setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/Mixer_MaimLevelKnob.svg")));
	shadow->box.pos = Vec(0.0, 2.5);
	minAngle = -0.5 * M_PI;
	maxAngle = 0.8 * M_PI;
}

MixerLevel::MixerLevel() {
	setBackgroundSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/Mixer_SliderPot.svg")));
	setHandleSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/Mixer_SliderHandle.svg")));
	maxHandlePos = mm2px(Vec(-.5, 0));
	minHandlePos = mm2px(Vec(-.5, 30));
	//background->box.pos = margin;
	//box.size = background->box.size.plus(margin.mult(2));
}

MixerVuMeter::MixerVuMeter() {
	box.size = mm2px(Vec(1.5, 34));
}

void MixerVuMeter::draw(const DrawArgs& args) {
	float y = module->params[Mixer_1::SLIDER_LEVEL_PARAM + 0].getValue();
	nvgStrokeColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0x10));
	{
		nvgBeginPath(args.vg);
		nvgMoveTo(args.vg, box.pos.x, box.pos.y);
		nvgLineTo(args.vg, box.pos.x, box.pos.y - box.size.y * y);
		nvgClosePath(args.vg);
	}
	nvgStrokeWidth(args.vg, 3.f);
	nvgStroke(args.vg);
}
