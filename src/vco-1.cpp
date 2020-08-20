#include "plugin.hpp"


struct VCO_1 : Module {
	enum ParamIds {
		TUNEKNOB_PARAM,
		PITCHKNOB_PARAM,
		OCTAVEKNOB_PARAM,
		PULSEWIDTHKNOB_PARAM,
		FMKNOB_PARAM,
		PWMKNOB_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		OCTAVEJACK_INPUT,
		TUNEJACK_INPUT,
		VOCTJACK_INPUT,
		FMJACK_INPUT,
		PWMJACK_INPUT,
		SYNCJACK_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		SINEWAVEJACK_OUTPUT,
		TRIANGLEWAVEJACK_OUTPUT,
		SAWWAVEJACK_OUTPUT,
		SQUAREWAVEJACK_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		BLINK_LIGHT,
		NUM_LIGHTS
	};

	float phase = 0.0;

	VCO_1() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(PITCHKNOB_PARAM, 0.f, 1.f, 0.5f, "Pitch");
		configParam(TUNEKNOB_PARAM, -1.f, 1.f, 0.f, "Tune pitch");
		configParam(OCTAVEKNOB_PARAM, -2.f, 2.f, 0.f, "Octave");
		configParam(FMKNOB_PARAM, 0.f, 1.f, 0.f, "Pitch modulation", "%", 0.f, 100.f);
		configParam(PULSEWIDTHKNOB_PARAM, 0.01f, 0.99f, 0.5f, "Pulse width", "%", 0.f, 100.f);
		configParam(PWMKNOB_PARAM, 0.f, 1.f, 0.f, "Pulse width modulation", "%", 0.f, 100.f);
	}

	void process(const ProcessArgs& args) override {
		float pitch = params[PITCHKNOB_PARAM].getValue();
		if (inputs[VOCTJACK_INPUT].isConnected()) {
			  pitch += inputs[VOCTJACK_INPUT].getVoltage();
		}
			  pitch += params[TUNEKNOB_PARAM].getValue(); 
		if (inputs[TUNEJACK_INPUT].isConnected()) {
			  pitch += inputs[TUNEJACK_INPUT].getVoltage(); 
		}
		float freq = dsp::FREQ_C4 + pitch;
			  freq = clamp(freq, 0.f, 20000.f);
		float deltaPhase = clamp(freq * args.sampleTime, 1e-6f, 0.5f);
		float oldPhase = phase;
			  phase += deltaPhase;

		float sine = -std::cos(2*M_PI * phase);
		if (outputs[SINEWAVEJACK_OUTPUT].isConnected()) {
			outputs[SINEWAVEJACK_OUTPUT].setVoltage(5.f*sine);
		}
	}
};


struct VCO_1Widget : ModuleWidget {
	VCO_1Widget(VCO_1* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/VCO-1.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<Trimpot>(mm2px(Vec(33.82, 14.013)), module, VCO_1::TUNEKNOB_PARAM));
		addParam(createParamCentered<RoundHugeBlackKnob>(mm2px(Vec(20.032, 19.746)), module, VCO_1::PITCHKNOB_PARAM));
		addParam(createParamCentered<RoundHugeBlackKnob>(mm2px(Vec(20.32, 49.976)), module, VCO_1::OCTAVEKNOB_PARAM));
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(20.32, 83.827)), module, VCO_1::PULSEWIDTHKNOB_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(6.82, 85.966)), module, VCO_1::FMKNOB_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(33.82, 85.966)), module, VCO_1::PWMKNOB_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.82, 35.54)), module, VCO_1::OCTAVEJACK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(33.82, 35.54)), module, VCO_1::TUNEJACK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.82, 103.574)), module, VCO_1::VOCTJACK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.82, 103.574)), module, VCO_1::FMJACK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24.82, 103.574)), module, VCO_1::PWMJACK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(33.82, 103.574)), module, VCO_1::SYNCJACK_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(6.82, 112.573)), module, VCO_1::SINEWAVEJACK_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.82, 112.573)), module, VCO_1::TRIANGLEWAVEJACK_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(24.82, 112.573)), module, VCO_1::SAWWAVEJACK_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(33.82, 112.573)), module, VCO_1::SQUAREWAVEJACK_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(22.926, 2.107)), module, VCO_1::BLINK_LIGHT));
	}
};


Model* modelVCO_1 = createModel<VCO_1, VCO_1Widget>("VCO-1");
