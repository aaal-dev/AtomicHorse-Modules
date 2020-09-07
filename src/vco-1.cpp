#include "vco-1.hpp"

VCO_1::VCO_1() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configParam<VCOFreqKnobParamQuantity>(KNOB_FREQ_PARAM, -6.0f, 6.0f, 0.0f, "Frequency", " Hz");
	configParam(KNOB_FM_PARAM, 0.f, 1.f, 0.f, "Frequency modulation depth", "%", 0.f, 100.f);
	configParam(KNOB_TUNE_PARAM, -1.f, 1.f, 0.f, "Fine tune");
	configParam(KNOB_OCTAVE_PARAM, -3, 2, 0, "Octave", "'");
	configParam(KNOB_PULSEWIDTH_PARAM, 0.99f, 0.01f, 0.5f, "Pulse width", "%", 0.f, 100.f);
	configParam(KNOB_PWM_PARAM, 0.f, 1.f, 0.f, "Pulse width modulation depth", "%", 0.f, 100.f);
	configParam(SWITCH_SIGNALTYPE_PARAM, 0.f, 1.f, 1.f, "Analog or digital signal type");
	configParam(SWITCH_SYNCMODE_PARAM, 0.f, 1.f, 1.f, "Hard or soft sync mode");
}

simd::float_4 VCO_1::sin2pi_pade_05_5_4(simd::float_4 x) {
	x -= 0.5f;
	return (simd::float_4(-6.283185307) * x + simd::float_4(33.19863968) * simd::pow(x, 3) - simd::float_4(32.44191367) * simd::pow(x, 5))
	/ (1 + simd::float_4(1.296008659) * simd::pow(x, 2) + simd::float_4(0.7028072946) * simd::pow(x, 4));
}

simd::float_4 VCO_1::expCurve(simd::float_4 x) {
	return (3 + x * (-13 + 5 * x)) / (3 + 2 * x);
}

simd::float_4 VCO_1::sinewave(simd::float_4 phase) {
	simd::float_4 sinewave;
	if (analogsignal) {
		simd::float_4 halfPhase = (phase < 0.5f);
		simd::float_4 x = phase - simd::ifelse(halfPhase, 0.25f, 0.75f);
		sinewave = 1.f - 16.f * simd::pow(x, 2);
		sinewave *= simd::ifelse(halfPhase, 1.f, -1.f);
	} else {
		sinewave = sin2pi_pade_05_5_4(phase);
	}
	return sinewave;
}

simd::float_4 VCO_1::trianglewave(simd::float_4 phase) {
	simd::float_4 trianglewave;
	if (analogsignal) {
		simd::float_4 x = phase + 0.25f;
		x -= simd::trunc(x);
		simd::float_4 halfX = (x >= 0.5f);
		x *= 2;
		x -= simd::trunc(x);
		trianglewave = expCurve(x) * simd::ifelse(halfX, 1.f, -1.f);
	} else {
		trianglewave = 1 - 4 * simd::fmin(simd::fabs(phase - 0.25f), simd::fabs(phase - 1.25f));
	}
	return trianglewave;
}

simd::float_4 VCO_1::sawwave(simd::float_4 phase) {
	simd::float_4 sawwave;
	simd::float_4 x = phase + 0.5f;
	x -= simd::trunc(x);
	if (analogsignal) {
		sawwave = -expCurve(x);
	} else {
		sawwave = 2 * x - 1;
	}
	return sawwave;
}

simd::float_4 VCO_1::squarewave(simd::float_4 phase, simd::float_4 pwParamValue) {
	return simd::ifelse(phase < pwParamValue, 1.f, -1.f);
}

void VCO_1::process(const ProcessArgs& args) {
	// Number of poly voices
	int voices = std::max(inputs[JACK_VOCT_INPUT].getChannels(), 1);
	outputs[JACK_SINEWAVE_OUTPUT].setChannels(voices);
	outputs[JACK_TRIANGLEWAVE_OUTPUT].setChannels(voices);
	outputs[JACK_SAWWAVE_OUTPUT].setChannels(voices);
	outputs[JACK_SQUAREWAVE_OUTPUT].setChannels(voices);

	// Analog or digital signal parameter
	analogsignal = params[SWITCH_SIGNALTYPE_PARAM].getValue() > 0.f;

	for (int voice = 0; voice < voices; voice += 4) {

		// Freguency parameter
		simd::float_4 freq_param = params[KNOB_FREQ_PARAM].getValue();
		if (inputs[JACK_FM_INPUT].isConnected()) {
			simd::float_4 fm_signal = inputs[JACK_FM_INPUT].getVoltageSimd<simd::float_4>(voice);
			simd::float_4 fm_param  = params[KNOB_FM_PARAM].getValue();
			freq_param += fm_signal * fm_param;
		}

		// Fine tune parameter
		simd::float_4 tune_param = dsp::quadraticBipolar(params[KNOB_TUNE_PARAM].getValue());
		if (inputs[JACK_TUNE_INPUT].isConnected()) {
			simd::float_4 tune_signal = inputs[JACK_TUNE_INPUT].getVoltageSimd<simd::float_4>(voice);
			tune_param += tune_signal / 12.f;
		}
		freq_param += tune_param;

		// Octave parameter
		float octave_param = params[KNOB_OCTAVE_PARAM].getValue();
		if (inputs[JACK_OCTAVE_INPUT].isConnected()) {
			float octave_signal = inputs[JACK_OCTAVE_INPUT].getVoltage();
			octave_signal = rescale(octave_signal, -3.f, 2.f, -3.f, 2.f);
			octave_param += octave_signal;
			octave_param = clamp(octave_param, -3.f, 2.f);
		}
		freq_param += (int)octave_param;

		// Pulse width parameter
		simd::float_4 pulsewidth_param = params[KNOB_PULSEWIDTH_PARAM].getValue();
		if (inputs[JACK_PWM_INPUT].isConnected()) {
			simd::float_4 pwm_signal = inputs[JACK_PWM_INPUT].getVoltageSimd<simd::float_4>(voice) / 10.f;
			simd::float_4 pwm_param = params[KNOB_PWM_PARAM].getValue();
			pwm_signal = rescale(pwm_signal, 0.f, 1.f, 0.f, 1.f);
			pulsewidth_param += pwm_signal * pwm_param;
		}
		pulsewidth_param = clamp(pulsewidth_param, .01f, .99f);

		if (inputs[JACK_VOCT_INPUT].isConnected()) {
			simd::float_4 voct_signal = inputs[JACK_VOCT_INPUT].getVoltageSimd<simd::float_4>(voice);
			freq_param += voct_signal;
		}

		simd::float_4 frequency = dsp::FREQ_C4 * dsp::approxExp2_taylor5(freq_param + 30) / 1073741824;
		simd::float_4 delta_phase = clamp(frequency * args.sampleTime, 1e-6f, 0.35f);
		phase[voice] += delta_phase;
		phase[voice] -= simd::floor(phase[voice]);

		simd::float_4 wrap_phase = (sync_direction[voice] == -1.f) & 1.f;
		simd::float_4 wrap_crossing = (wrap_phase - (phase[voice] - delta_phase)) / delta_phase;
		int wrap_mask = simd::movemask((0 < wrap_crossing) & (wrap_crossing <= 1.f));
		if (wrap_mask) {
			for (int i = 0; i < std::min(voices - voice, 4); i++) {
				if (wrap_mask & (1 << i)) {
					simd::float_4 mask = simd::movemaskInverse<simd::float_4>(1 << i);
					float p = wrap_crossing[i] - 1.f;
					simd::float_4 x = mask & (2.f * sync_direction[voice]);
					squarewave_blep[voice].insertDiscontinuity(p, x);
				}
			}
		}

		simd::float_4 pulse_crossing = (pulsewidth_param - (phase[voice] - delta_phase)) / delta_phase;
		int pulse_mask = simd::movemask((0 < pulse_crossing) & (pulse_crossing <= 1.f));
		if (pulse_mask) {
			for (int i = 0; i < std::min(voices - voice, 4); i++) {
				if (pulse_mask & (1 << i)) {
					simd::float_4 mask = simd::movemaskInverse<simd::float_4>(1 << i);
					float p = pulse_crossing[i] - 1.f;
					simd::float_4 x = mask & (-2.f * sync_direction[voice]);
					squarewave_blep[voice].insertDiscontinuity(p, x);
				}
			}
		}

		simd::float_4 half_crossing = (0.5f - (phase[voice] - delta_phase)) / delta_phase;
		int half_mask = simd::movemask((0 < half_crossing) & (half_crossing <= 1.f));
		if (half_mask) {
			for (int i = 0; i < std::min(voices - voice, 4); i++) {
				if (half_mask & (1 << i)) {
					simd::float_4 mask = simd::movemaskInverse<simd::float_4>(1 << i);
					float p = half_crossing[i] - 1.f;
					simd::float_4 x = mask & (-2.f * sync_direction[voice]);
					sawwave_blep[voice].insertDiscontinuity(p, x);
				}
			}
		}

		// Synchronization
		if (inputs[JACK_SYNC_INPUT].isConnected()) {
			simd::float_4 sync_signal = inputs[JACK_SYNC_INPUT].getPolyVoltageSimd<simd::float_4>(voice);
			simd::float_4 delta_sync = sync_signal - last_sync[voice];
			simd::float_4 sync_crossing = -last_sync[voice] / delta_sync;
			last_sync[voice] = sync_signal;
			simd::float_4 sync_param = (0.f < sync_crossing) & (sync_crossing <= 1.f) & (sync_signal >= 0.f);
			int sync_mask = simd::movemask(sync_param);
			if (sync_mask) {
				if (params[SWITCH_SYNCMODE_PARAM].getValue() <= 0.f) {
					sync_direction[voice] = simd::ifelse(sync_param, -sync_direction[voice], sync_direction[voice]);
				} else {
					simd::float_4 new_phase = simd::ifelse(sync_param, (1.f - sync_crossing) * delta_phase, phase[voice]);
					// Insert minBLEP for sync
					for (int i = 0; i < std::min(voices - voice, 4); i++) {
						if (sync_mask & (1 << i)) {
							simd::float_4 mask = simd::movemaskInverse<simd::float_4>(1 << i);
							float p = sync_crossing[i] - 1.f;
							simd::float_4 x;
							x = mask & (sinewave(new_phase) - sinewave(phase[voice]));
							sinewave_blep[voice].insertDiscontinuity(p, x);
							x = mask & (trianglewave(new_phase) - trianglewave(phase[voice]));
							trianglewave_blep[voice].insertDiscontinuity(p, x);
							x = mask & (sawwave(new_phase) - sawwave(phase[voice]));
							sawwave_blep[voice].insertDiscontinuity(p, x);
							x = mask & (squarewave(new_phase, pulsewidth_param) - squarewave(phase[voice], pulsewidth_param));
							squarewave_blep[voice].insertDiscontinuity(p, x);
						}
					}
					phase[voice] = new_phase;
				}
			}
		}

		// Sinewave
		if (outputs[JACK_SINEWAVE_OUTPUT].isConnected()) {
			simd::float_4 sinewave = this->sinewave(phase[voice]);
			sinewave += sinewave_blep[voice].process();
			outputs[JACK_SINEWAVE_OUTPUT].setVoltageSimd(5.f*sinewave, voice);
		}

		// Trianglewave
		if (outputs[JACK_TRIANGLEWAVE_OUTPUT].isConnected()) {
			simd::float_4 trianglewave = this->trianglewave(phase[voice]);
			trianglewave += trianglewave_blep[voice].process();
			outputs[JACK_TRIANGLEWAVE_OUTPUT].setVoltageSimd(5.f*trianglewave, voice);
		}

		// Sawwave
		if (outputs[JACK_SAWWAVE_OUTPUT].isConnected()) {
			simd::float_4 sawwave = this->sawwave(phase[voice]);
			sawwave += sawwave_blep[voice].process();
			outputs[JACK_SAWWAVE_OUTPUT].setVoltageSimd(5.f*sawwave, voice);
		}

		// Squarewave
		if (outputs[JACK_SQUAREWAVE_OUTPUT].isConnected()) {
			simd::float_4 squarewave = this->squarewave(phase[voice], pulsewidth_param);
			squarewave += squarewave_blep[voice].process();
			if (analogsignal) {
				sqr_filter[voice].setCutoffFreq(20.f * args.sampleTime);
				sqr_filter[voice].process(squarewave);
				squarewave = sqr_filter[voice].highpass() * 0.95f;
			}
			outputs[JACK_SQUAREWAVE_OUTPUT].setVoltageSimd(5.f*squarewave, voice);
		}
	}
}


VCO_1Widget::VCO_1Widget(VCO_1* module) {
	setModule(module);
	setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/VCO-1.svg")));

	addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	addParam(createParamCentered<CKSS>(mm2px(Vec(15.82, 99.574)), module, VCO_1::SWITCH_SIGNALTYPE_PARAM));
	addParam(createParamCentered<CKSS>(mm2px(Vec(24.82, 99.574)), module, VCO_1::SWITCH_SYNCMODE_PARAM));

	addParam(createParamCentered<AHMRoundKnobWhiteTiny>(mm2px(Vec(33.82, 13.0)), module, VCO_1::KNOB_TUNE_PARAM));
	addParam(createParamCentered<VCOFrequencyKnob>(mm2px(Vec(20.32, 23.0)), module, VCO_1::KNOB_FREQ_PARAM));
	addParam(createParamCentered<VCOOctaveRotaryTumbler>(mm2px(Vec(20.32, 51.785)), module, VCO_1::KNOB_OCTAVE_PARAM));
	addParam(createParamCentered<AHMRoundKnobWhiteLarge>(mm2px(Vec(20.32, 75.525)), module, VCO_1::KNOB_PULSEWIDTH_PARAM));
	addParam(createParamCentered<AHMRoundKnob2WhiteTiny>(mm2px(Vec(6.82, 13.0)), module, VCO_1::KNOB_FM_PARAM));
	addParam(createParamCentered<AHMRoundKnob3WhiteTiny>(mm2px(Vec(6.82, 75.525)), module, VCO_1::KNOB_PWM_PARAM));

	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.82, 51.785)), module, VCO_1::JACK_OCTAVE_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(33.82, 33.0)), module, VCO_1::JACK_TUNE_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.82, 103.574)), module, VCO_1::JACK_VOCT_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.82, 33.0)), module, VCO_1::JACK_FM_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(33.82, 75.525)), module, VCO_1::JACK_PWM_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(33.82, 103.574)), module, VCO_1::JACK_SYNC_INPUT));

	addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(6.82, 112.573)), module, VCO_1::JACK_SINEWAVE_OUTPUT));
	addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.82, 112.573)), module, VCO_1::JACK_TRIANGLEWAVE_OUTPUT));
	addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(24.82, 112.573)), module, VCO_1::JACK_SAWWAVE_OUTPUT));
	addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(33.82, 112.573)), module, VCO_1::JACK_SQUAREWAVE_OUTPUT));

	addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(2.32, 108.073)), module, VCO_1::LED_BLINK_LIGHT));
}

Model* modelVCO_1 = createModel<VCO_1, VCO_1Widget>("VCO-1");

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

VCOFrequencyKnob::VCOFrequencyKnob() {
	setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/VCO_FreqKnob.svg")));
	shadow->box.pos = Vec(0.0, 2.5);
	minAngle = -0.8 * M_PI;
	maxAngle = 0.8 * M_PI;
}

VCOOctaveRotaryTumbler::VCOOctaveRotaryTumbler() {
	setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/VCO_OctaveRotaryTumbler.svg")));
	minAngle = -0.3 * M_PI;
    maxAngle = 0.3 * M_PI;
	snap = true;
	shadow->opacity = 0.f;
}
