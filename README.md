# AtomicHorse Modules v1.0.0 for [VCV Rack](https://github.com/VCVRack/)

Some C++ self-learning and fun time with some math. Do some design stuff. And maybe use it in future for music making with VCV Rack.

I use some free fonts from [FontSpace](https://www.fontspace.com/commercial-fonts):

\- [Big Noodle Titling Font](https://www.fontspace.com/big-noodle-titling-font-f35093) by [Sentinel Type](https://www.fontspace.com/sentinel-type) in VCO-1, Envelope-1, LFO-1, Mixer-1

VCO-1 | Envelope-1 | LFO-1 | Mixer | VCF
|---|---|---|---|---|
| ![VCO-1](/pics/vco-1.png "Third version of visual design") | ![Envelope-1](/pics/envelope-1.png "Third version of visual design") | ![LFO-1](/pics/lfo-1.png "Second version of visual design") | ![Mixer-1](/pics/mixer-1.png "First version of visual design") | Not done yet |

## VCO-1

First take of VCO. I want some extra things in VCO, but can't find it on other. That's why I start this. I know that's all can be done with VCV Fundament collection of modules. But I want some narrowed panel for eye-pick look.

> It is working, but have some issues with polyphony. Only 4 voices is functioning right for now.

## Envelope-1

First take of Envelope. Enhanced version of typical ADSR with start time, target level, hold time and delay time. And I have emplemented slope controls of attack, decay and release. It is like DAHDSR, but I saw that sustain in most cases means level of signal, not a lenght. But sometimes lenght of sustain might be useful. That's why I put delay after decay as lenght of sustain. And for preventing misunderstunding with start delay, I named it just Start. 

I use rescale for knobs. I think it's more helpful. 

> Now it is working. Polyphonic on 16 voices, but without simd. Can be slow with many voices. Need some clamps in code, and new visual design with numbers.

## LFO-1

First take of LFO. I want some extra controls. And I think it would be much better with waveform changer and level knob on each output. Also I want to emplement frequency changer on each output.

> Not working. Only skeleton for visual design.  


## Mixer-1

First take of standart 4 track mixer. I made code flexible. Only need is big panel and change TRACKS_NUMBER in [mixer-1.hpp](https://github.com/animeslave/AtomicHorseModules/blob/master/src/mixer-1.hpp). It is polyphonic and stereo. I need change visual design. And I want VuMeters on tracks, but I don't know how to do them yet. 

> Working. Panning is not working yet. Beware of loud volume. Not tuned.

