#include "vco-1.hpp"

VCO_1::VCO_1() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configParam(SIGNALTYPE_PARAM, 0.f, 1.f, 1.f, "Analog or digital signal type");
	configParam(SYNCMODE_PARAM, 0.f, 1.f, 1.f, "Hard or soft sync mode");
	configParam<VCOFreqKnobParamQuantity>(FREQKNOB_PARAM, -3.0f, 6.0f, 0.0f, "Frequency", " Hz");
	configParam(TUNEKNOB_PARAM, -1.f, 1.f, 0.f, "Fine tune");
	configParam(OCTAVEKNOB_PARAM, -3, 2, 0, "Octave", "'");
	configParam(FMKNOB_PARAM, 0.f, 1.f, 0.f, "Frequency modulation depth", "%", 0.f, 100.f);
	configParam(PULSEWIDTHKNOB_PARAM, 0.99f, 0.01f, 0.5f, "Pulse width", "%", 0.f, 100.f);
	configParam(PWMKNOB_PARAM, 0.f, 1.f, 0.f, "Pulse width modulation depth", "%", 0.f, 100.f);
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

simd::float_4 VCO_1::squarewave(simd::float_4 phase, float pwParamValue) {
	return simd::ifelse(phase < pwParamValue, 1.f, -1.f);
}

void VCO_1::process(const ProcessArgs& args) {
	// Freguency parameter
	simd::float_4 freqParamValue = params[FREQKNOB_PARAM].getValue();
	if (inputs[FMJACK_INPUT].isConnected()) {
		float fmSignalValue = inputs[FMJACK_INPUT].getVoltage();
		float fmParamValue = params[FMKNOB_PARAM].getValue();
		freqParamValue += fmSignalValue * fmParamValue;
	}

	// Fine tune parameter
	float tuneParamValue = dsp::quadraticBipolar(params[TUNEKNOB_PARAM].getValue());
	if (inputs[TUNEJACK_INPUT].isConnected()) {
		float tuneSignalValue = inputs[TUNEJACK_INPUT].getVoltage();
		tuneParamValue += tuneSignalValue / 12.f;
	}
	freqParamValue += tuneParamValue;

	// Octave parameter
	float octaveParamValue = params[OCTAVEKNOB_PARAM].getValue();
	if (inputs[OCTAVEJACK_INPUT].isConnected()) {
		float octaveSignalValue = inputs[OCTAVEJACK_INPUT].getVoltage();
		octaveSignalValue = rescale(octaveSignalValue, -3.f, 2.f, -3.f, 2.f);
		octaveParamValue += octaveSignalValue;
		octaveParamValue = clamp(octaveParamValue, -3.f, 2.f);
	}
	freqParamValue += (int)octaveParamValue;

	// Pulse width parameter
	float pwParamValue = params[PULSEWIDTHKNOB_PARAM].getValue();
	if (inputs[PWMJACK_INPUT].isConnected()) {
		float pwmSignalValue = inputs[PWMJACK_INPUT].getVoltage() / 10.f;
		pwmSignalValue = rescale(pwmSignalValue, 0.f, 1.f, 0.f, 1.f);
		float pwmParamValue = params[PWMKNOB_PARAM].getValue();
		pwParamValue += pwmSignalValue * pwmParamValue;
	}
	pwParamValue = clamp(pwParamValue, .01f, .99f);

	// Analog or digital signal parameter
	analogsignal = params[SIGNALTYPE_PARAM].getValue() > 0.f;



	int channels = std::max(inputs[VOCTJACK_INPUT].getChannels(), 1);
	for (int channel = 0; channel < channels; channel += 4) {
		if (inputs[VOCTJACK_INPUT].isConnected()) {
			simd::float_4 voctSignalValue = inputs[VOCTJACK_INPUT].getVoltageSimd<simd::float_4>(channel);
			freqParamValue += voctSignalValue;
		}

		simd::float_4 freq = dsp::FREQ_C4 * dsp::approxExp2_taylor5(freqParamValue + 30) / 1073741824;
		simd::float_4 deltaPhase = clamp(freq * args.sampleTime, 1e-6f, 0.35f);
		phase += deltaPhase;
		phase -= simd::floor(phase);

		simd::float_4 wrapPhase = (syncDirection == -1.f) & 1.f;
		simd::float_4 wrapCrossing = (wrapPhase - (phase - deltaPhase)) / deltaPhase;
		int wrapMask = simd::movemask((0 < wrapCrossing) & (wrapCrossing <= 1.f));
		if (wrapMask) {
			for (int i = 0; i < std::min(channels - channel, 4); i++) {
				if (wrapMask & (1 << i)) {
					simd::float_4 mask = simd::movemaskInverse<simd::float_4>(1 << i);
					float p = wrapCrossing[i] - 1.f;
					simd::float_4 x = mask & (2.f * syncDirection);
					squarewaveMinBlep.insertDiscontinuity(p, x);
				}
			}
		}

		simd::float_4 pulseCrossing = (pwParamValue - (phase - deltaPhase)) / deltaPhase;
		int pulseMask = simd::movemask((0 < pulseCrossing) & (pulseCrossing <= 1.f));
		if (pulseMask) {
			for (int i = 0; i < std::min(channels - channel, 4); i++) {
				if (pulseMask & (1 << i)) {
					simd::float_4 mask = simd::movemaskInverse<simd::float_4>(1 << i);
					float p = pulseCrossing[i] - 1.f;
					simd::float_4 x = mask & (-2.f * syncDirection);
					squarewaveMinBlep.insertDiscontinuity(p, x);
				}
			}
		}

		simd::float_4 halfCrossing = (0.5f - (phase - deltaPhase)) / deltaPhase;
		int halfMask = simd::movemask((0 < halfCrossing) & (halfCrossing <= 1.f));
		if (halfMask) {
			for (int i = 0; i < std::min(channels - channel, 4); i++) {
				if (halfMask & (1 << i)) {
					simd::float_4 mask = simd::movemaskInverse<simd::float_4>(1 << i);
					float p = halfCrossing[i] - 1.f;
					simd::float_4 x = mask & (-2.f * syncDirection);
					sawwaveMinBlep.insertDiscontinuity(p, x);
				}
			}
		}

		// Synchronization
		if (inputs[SYNCJACK_INPUT].isConnected()) {
			simd::float_4 syncSignalValue = inputs[SYNCJACK_INPUT].getPolyVoltageSimd<simd::float_4>(channel);
			simd::float_4 deltaSync = syncSignalValue - lastSyncValue;
			simd::float_4 syncCrossing = -lastSyncValue / deltaSync;
			lastSyncValue = syncSignalValue;
			simd::float_4 syncParamValue = (0.f < syncCrossing) & (syncCrossing <= 1.f) & (syncSignalValue >= 0.f);
			int syncMask = simd::movemask(syncParamValue);
			if (syncMask) {
				if (params[SYNCMODE_PARAM].getValue() <= 0.f) {
					syncDirection = simd::ifelse(syncParamValue, -syncDirection, syncDirection);
				} else {
					simd::float_4 newPhase = simd::ifelse(syncParamValue, (1.f - syncCrossing) * deltaPhase, phase);
					// Insert minBLEP for sync
					for (int i = 0; i < std::min(channels - channel, 4); i++) {
						if (syncMask & (1 << i)) {
							simd::float_4 mask = simd::movemaskInverse<simd::float_4>(1 << i);
							float p = syncCrossing[i] - 1.f;
							simd::float_4 x;
							x = mask & (sinewave(newPhase) - sinewave(phase));
							sinewaveMinBlep.insertDiscontinuity(p, x);
							x = mask & (trianglewave(newPhase) - trianglewave(phase));
							trianglewaveMinBlep.insertDiscontinuity(p, x);
							x = mask & (sawwave(newPhase) - sawwave(phase));
							sawwaveMinBlep.insertDiscontinuity(p, x);
							x = mask & (squarewave(newPhase, pwParamValue) - squarewave(phase, pwParamValue));
							squarewaveMinBlep.insertDiscontinuity(p, x);
						}
					}
					phase = newPhase;
				}
			}
		}

		// Sinewave
		if (outputs[SINEWAVEJACK_OUTPUT].isConnected()) {
			simd::float_4 sinewave = this->sinewave(phase);
			sinewave += sinewaveMinBlep.process();
			outputs[SINEWAVEJACK_OUTPUT].setChannels(channels);
			outputs[SINEWAVEJACK_OUTPUT].setVoltageSimd(5.f*sinewave, channel);
		}

		// Trianglewave
		if (outputs[TRIANGLEWAVEJACK_OUTPUT].isConnected()) {
			simd::float_4 trianglewave = this->trianglewave(phase);
			trianglewave += trianglewaveMinBlep.process();
			outputs[TRIANGLEWAVEJACK_OUTPUT].setChannels(channels);
			outputs[TRIANGLEWAVEJACK_OUTPUT].setVoltageSimd(5.f*trianglewave, channel);
		}

		// Sawwave
		if (outputs[SAWWAVEJACK_OUTPUT].isConnected()) {
			simd::float_4 sawwave = this->sawwave(phase);
			sawwave += sawwaveMinBlep.process();
			outputs[SAWWAVEJACK_OUTPUT].setChannels(channels);
			outputs[SAWWAVEJACK_OUTPUT].setVoltageSimd(5.f*sawwave, channel);
		}

		// Squarewave
		if (outputs[SQUAREWAVEJACK_OUTPUT].isConnected()) {
			simd::float_4 squarewave = this->squarewave(phase, pwParamValue);
			squarewave += squarewaveMinBlep.process();
			if (analogsignal) {
				sqrFilter.setCutoffFreq(20.f * args.sampleTime);
				sqrFilter.process(squarewave);
				squarewave = sqrFilter.highpass() * 0.95f;
			}
			outputs[SQUAREWAVEJACK_OUTPUT].setChannels(channels);
			outputs[SQUAREWAVEJACK_OUTPUT].setVoltageSimd(5.f*squarewave, channel);
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

	addParam(createParamCentered<CKSS>(mm2px(Vec(15.82, 99.574)), module, VCO_1::SIGNALTYPE_PARAM));
	addParam(createParamCentered<CKSS>(mm2px(Vec(24.82, 99.574)), module, VCO_1::SYNCMODE_PARAM));

	addParam(createParamCentered<AHMRoundKnobWhiteTiny>(mm2px(Vec(33.82, 13.0)), module, VCO_1::TUNEKNOB_PARAM));
	addParam(createParamCentered<AHMRoundKnob2WhiteHuge>(mm2px(Vec(20.32, 23.0)), module, VCO_1::FREQKNOB_PARAM));
	addParam(createParamCentered<AHMRotaryTumblerWhiteHugeSnap>(mm2px(Vec(20.32, 51.785)), module, VCO_1::OCTAVEKNOB_PARAM));
	addParam(createParamCentered<AHMRoundKnobWhiteLarge>(mm2px(Vec(20.32, 75.525)), module, VCO_1::PULSEWIDTHKNOB_PARAM));
	addParam(createParamCentered<AHMRoundKnob2WhiteTiny>(mm2px(Vec(6.82, 13.0)), module, VCO_1::FMKNOB_PARAM));
	addParam(createParamCentered<AHMRoundKnob3WhiteTiny>(mm2px(Vec(6.82, 75.525)), module, VCO_1::PWMKNOB_PARAM));

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

AHMRotaryTumblerWhiteHugeSnap::AHMRotaryTumblerWhiteHugeSnap() {
	setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/AHMRotaryTumblerWhiteHuge.svg")));
	minAngle = -0.3 * M_PI;
    maxAngle = 0.3 * M_PI;
	snap = true;
	shadow->opacity = 0.f;
}
