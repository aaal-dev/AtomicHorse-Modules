# AtomicHorse Modules v1.0.0 for [VCV Rack](https://github.com/VCVRack/)

Some C++ self-learning and *fun* time with some math. Do some design stuff. And maybe use it in future for music making with VCV Rack.

I use some free fonts from [FontSpace](https://www.fontspace.com/commercial-fonts):

\- [Big Noodle Titling Font](https://www.fontspace.com/big-noodle-titling-font-f35093) by [Sentinel Type](https://www.fontspace.com/sentinel-type)


## Oscilators

| [VCO-1](#vco-1)           | [LFO-1](#lfo-1)           |
| ---                       | ---                       |
| ![VCO-1](/pics/vco-1.png) | ![LFO-1](/pics/lfo-1.png) |


### VCO-1

| Status | Polyphony     |
| ---    | ---           |
| Work   | Yes (w/ simd) |

First take of VCO. I want some extra things in VCO from Fundamental Modules, but can't find it on other VCOs. That's why I start this. I know that's all can be done with Fundamental Modules. But I want some narrowed panel for eye-pick look.


### LFO-1

| Status | Polyphony |
| ---    | ---       |
| WIP    | -         |

First take of LFO. I want some extra controls. And I think it would be much better with waveform changer and level knob on each output. Also I want to emplement frequency changer on each output.

Only skeleton for visual design for now. 


## Envelopes

| [Envelope-1](#envelope-1)           | [ADSR](#adsr)           |
| ---                                 | ---                     |
| ![Envelope-1](/pics/envelope-1.png) | ![ADSR](/pics/adsr.png) |


### Envelope-1

| Status | Polyphony      |
| ---    | ---            |
| Work   | Yes (w/o simd) |

Enhanced version of typical ADSR with start time, target level, hold time and delay time. And I have emplemented slope controls of attack, decay and release. It's like DAHDSR, but I saw that sustain in most cases means level of signal, not a lenght. But sometimes lenght of sustain might be useful. That's why I put delay after decay as lenght of sustain. And for preventing misunderstunding with start delay, I named it just Start. 

I use rescale for knobs, I think it's might be helpful. Need some clamps in code, and new visual design with numbers.


### ADSR

| Status | Polyphony      |
| ---    | ---            |
| Work   | Yes (w/o simd) |

Standart ADSR envelope generation. Simplified version of Envelope-1. Attack is linear. Decay and Release is logarythmical.


## Mixers

| [Mixer-1](#mixer-1)           | [2Mto1S](#2Mto1S)                | [4Mto1M](#4Mto1M)                | [4Sto1S](#4Sto1S) |
| ---                           | ---                              | ---                              | ---               |
| ![Mixer-1](/pics/mixer-1.png) | ![2Mto1S](/pics/mixer-2m-1s.png) | ![4Mto1M](/pics/mixer-4m-1m.png) | *WIP*             |

### Mixer-1

| Status | Polyphony      |
| ---    | ---            |
| Work   | Yes (w/o simd) |

**Beware of loud volume. Not tuned yet.**

First take of standart 4 track mixer. I want stereo mixer with behaviors like in Mixer from Fundamental Modules. I made code flexible. Only need is big panel and change TRACKS_NUMBER in [mixer-1.hpp](src/mixer-1.hpp). It's polyphonic and stereo with panning. I need change visual design. And I want VuMeters on tracks, but I don't know how to do them yet. 

### 2Mto1S

| Status | Polyphony      |
| ---    | ---            |
| Work   | Yes (w/o simd) |

Simple mixer of 2 mono signal to stereo with panning controls for each of two signals. 

I implemented volume compensation. I thing it's might be helpful. I don't read any theoretical thing about it, I got it by myself, and maybe I did it wrong, but result for me is enough. Maybe sometime I will got it right.

### 4Mto1M

| Status | Polyphony      |
| ---    | ---            |
| Work   | Yes (w/o simd) |

Standart 4 channel mono mixer like Mixer from Fundamental Modules, but with volume compensation. The point is code is flexible. It can be extendent to any number of input. Only need is make bigger panel .svg file and change TRACKS_NUMBER in [mixer-4m-1m.hpp](src/mixer-4m-1m.hpp). 

### 4Sto1S

| Status | Polyphony      |
| ---    | ---            |
| WIP    | -              |

Reimaging of Mixer-1. 4 channel stereo mixer with behaviors like in Mixer from Fundamental Modules. Like in 4Mto1M code is flexible. It can be extendent to any number of stereo channels. Only need is make bigger panel .svg file and change TRACKS_NUMBER in [mixer-4s-1s.hpp](src/mixer-4s-1s.hpp). 
