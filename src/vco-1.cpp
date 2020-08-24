#include "plugin.hpp"



struct VCO_1 : Module { 
	enum ParamIds {
		TUNEKNOB_PARAM,
		FREQKNOB_PARAM,
		OCTAVEKNOB_PARAM,
		PULSEWIDTHKNOB_PARAM,
		FMKNOB_PARAM,
		PWMKNOB_PARAM,
		WAVETYPE_PARAM,
		SYNCMODE_PARAM,
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
	
	simd::float_4 phase = 0.f;
	float sync = 0.f;
	float octave = 0.f;
	float tune = 0.f;
	simd::float_4 pitch = 0.f;
	float pulseWidth = .5f; 
	
	dsp::MinBlepGenerator<16, 32, float> sinMinBlep;
	dsp::MinBlepGenerator<16, 32, float> triMinBlep;
	dsp::MinBlepGenerator<16, 32, float> sawMinBlep;
	dsp::MinBlepGenerator<16, 32, float> sqrMinBlep;
	
	VCO_1() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(WAVETYPE_PARAM, 0.f, 1.f, 1.f, "Analog mode");
		configParam(SYNCMODE_PARAM, 0.f, 1.f, 1.f, "Hard sync");
		configParam<VCOFreqKnobParamQuantity>(FREQKNOB_PARAM, -3.0f, 6.0f, 0.0f, "Frequency", " Hz");
		configParam(TUNEKNOB_PARAM, -1.f, 1.f, 0.f, "Tune pitch");
		configParam(OCTAVEKNOB_PARAM, -3, 2, 0, "Octave", "'");
		configParam(FMKNOB_PARAM, 0.f, 1.f, 0.f, "Pitch modulation", "%", 0.f, 100.f);
		configParam(PULSEWIDTHKNOB_PARAM, 0.01f, 0.99f, 0.5f, "Pulse width", "%", 0.f, 100.f);
		configParam(PWMKNOB_PARAM, 0.f, 1.f, 0.f, "Pulse width modulation", "%", 0.f, 100.f);
	} 
	
	simd::float_4 sin2pi_pade_05_5_4(simd::float_4 x) {
		x -= 0.5f;
		return (simd::float_4(-6.283185307) * x + simd::float_4(33.19863968) * simd::pow(x, 3) - simd::float_4(32.44191367) * simd::pow(x, 5))
	       / (1 + simd::float_4(1.296008659) * simd::pow(x, 2) + simd::float_4(0.7028072946) * simd::pow(x, 4));
	}
		
	void process(const ProcessArgs& args) override {
		
		simd::float_4 freqParam = params[FREQKNOB_PARAM].getValue() / 12.f;
		if (inputs[FMJACK_INPUT].isConnected()) {
			simd::float_4 fmParam = dsp::quadraticBipolar(params[FMKNOB_PARAM].getValue());
			freqParam += inputs[FMJACK_INPUT].getVoltage() * fmParam;
		}
		
		float tuneParam = dsp::quadraticBipolar(params[TUNEKNOB_PARAM].getValue());
		if (inputs[TUNEJACK_INPUT].isConnected()) { 
			tuneParam += inputs[TUNEJACK_INPUT].getVoltage() / 12.f; 
		}
		
		freqParam += tuneParam;
		
		float octaveParam = params[OCTAVEKNOB_PARAM].getValue();		
		if (inputs[OCTAVEJACK_INPUT].isConnected()) { 
			octaveParam += inputs[OCTAVEJACK_INPUT].getVoltage();
			octaveParam = clamp(octaveParam, -3.f, 2.f); 
		}
		
		
		float pwParam = params[PULSEWIDTHKNOB_PARAM].getValue();		
		if (inputs[PWMJACK_INPUT].isConnected()) { 
			pwParam += inputs[PWMJACK_INPUT].getVoltage() * params[PWMKNOB_PARAM].getValue();
		}
		pwParam = rescale(clamp(pwParam, -1.f, 1.f), -1.f, 1.f, .05f, 1.f - .05f);
		
		int channels = std::max(inputs[VOCTJACK_INPUT].getChannels(), 1);
		for (int c = 0; c < channels; c += 4) {
			if (inputs[VOCTJACK_INPUT].isConnected()) {
				freqParam += inputs[VOCTJACK_INPUT].getVoltageSimd<simd::float_4>(c);
			}
		
			freqParam += (int)octaveParam;
		
			pitch = freqParam;
			simd::float_4 freq = dsp::FREQ_C4 * dsp::approxExp2_taylor5(pitch + 30) / 1073741824;
			//freq = dsp::FREQ_C4 * std::pow(2.f, pitch);
		
			simd::float_4 deltaPhase = clamp(freq * args.sampleTime, 1e-6f, 0.35f);
			phase += deltaPhase;
			phase -= simd::floor(phase);
		
			//float sinewave = std::cos(2*M_PI * phase);
			simd::float_4 sinewave = sin2pi_pade_05_5_4(phase);
			sinewave += sinMinBlep.process();
			
			simd::float_4 trianglewave = 1 - 4 * simd::fmin(simd::fabs(phase - 0.25f), simd::fabs(phase - 1.25f));
			trianglewave += triMinBlep.process();
			
			simd::float_4 sawwave = phase + 0.5f;
			sawwave -= simd::trunc(sawwave);
			sawwave = 2 * sawwave - 1;
			sawwave += sawMinBlep.process();
			
			simd::float_4 squarewave = simd::ifelse(phase < pwParam, 1.f, -1.f);
			squarewave += sqrMinBlep.process();
					
			if (outputs[SINEWAVEJACK_OUTPUT].isConnected()) {
				outputs[SINEWAVEJACK_OUTPUT].setChannels(channels);
				outputs[SINEWAVEJACK_OUTPUT].setVoltageSimd(5.f*sinewave, c);
			}
			if (outputs[TRIANGLEWAVEJACK_OUTPUT].isConnected()) {
				outputs[TRIANGLEWAVEJACK_OUTPUT].setChannels(channels);
				outputs[TRIANGLEWAVEJACK_OUTPUT].setVoltageSimd(5.f*trianglewave, c);
			}
			if (outputs[SAWWAVEJACK_OUTPUT].isConnected()) {
				outputs[SAWWAVEJACK_OUTPUT].setChannels(channels);
				outputs[SAWWAVEJACK_OUTPUT].setVoltageSimd(5.f*sawwave, c);
			}
			if (outputs[SQUAREWAVEJACK_OUTPUT].isConnected()) {
				outputs[SQUAREWAVEJACK_OUTPUT].setChannels(channels);
				outputs[SQUAREWAVEJACK_OUTPUT].setVoltageSimd(5.f*squarewave, c);
			}
		}
	}
};
	
struct VCO_1Widget : ModuleWidget {
	VCO_1Widget(VCO_1* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/VCO-1.svg")));
		
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		
		addParam(createParamCentered<CKSS>(mm2px(Vec(15.82, 99.574)), module, VCO_1::WAVETYPE_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(24.82, 99.574)), module, VCO_1::SYNCMODE_PARAM));
		
		addParam(createParamCentered<AHMRoundKnobWhiteTiny>(mm2px(Vec(33.82, 13.0)), module, VCO_1::TUNEKNOB_PARAM));
		addParam(createParamCentered<AHMRoundKnob2WhiteHuge>(mm2px(Vec(20.32, 23.0)), module, VCO_1::FREQKNOB_PARAM));
		addParam(createParamCentered<AHMRotaryTumblerWhiteHugeSnap>(mm2px(Vec(20.32, 51.785)), module, VCO_1::OCTAVEKNOB_PARAM));
		addParam(createParamCentered<AHMRoundKnobWhiteLarge>(mm2px(Vec(20.32, 75.525)), module, VCO_1::PULSEWIDTHKNOB_PARAM));
		addParam(createParamCentered<AHMRoundKnobWhiteTiny>(mm2px(Vec(6.82, 13.0)), module, VCO_1::FMKNOB_PARAM));
		addParam(createParamCentered<AHMRoundKnobWhiteTiny>(mm2px(Vec(6.82, 75.525)), module, VCO_1::PWMKNOB_PARAM));
		
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.82, 51.785)), module, VCO_1::OCTAVEJACK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(33.82, 33.0)), module, VCO_1::TUNEJACK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.82, 103.574)), module, VCO_1::VOCTJACK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.82, 33.0)), module, VCO_1::FMJACK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(33.82, 75.525)), module, VCO_1::PWMJACK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(33.82, 103.574)), module, VCO_1::SYNCJACK_INPUT));
		
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(6.82, 112.573)), module, VCO_1::SINEWAVEJACK_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.82, 112.573)), module, VCO_1::TRIANGLEWAVEJACK_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(24.82, 112.573)), module, VCO_1::SAWWAVEJACK_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(33.82, 112.573)), module, VCO_1::SQUAREWAVEJACK_OUTPUT));
		
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(2.32, 108.073)), module, VCO_1::BLINK_LIGHT));
	}
};

Model* modelVCO_1 = createModel<VCO_1, VCO_1Widget>("VCO-1");
