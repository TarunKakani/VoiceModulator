# Voice Modulator

Voice Modulator is a real-time voice modulation DSP pipeline written in C++. It uses PortAudio for live microphone input and output, then runs the signal through a small chain of math-heavy effects to change the perceived voice character on the fly. Inspired by batfleck.

## What it does

- Captures microphone audio in a PortAudio callback.
- Processes each audio block through a custom DSP chain.
- Applies pitch shifting, soft saturation, wave folding, low-pass smoothing, and a final limiter.
- Includes WAV loading and saving helpers for offline audio handling.

## How it works

The current live engine is centered in [src/main.cpp](src/main.cpp). PortAudio opens a mono input/output stream, and each buffer is passed to `DspChain::processVoice()` in [src/DspChain.cpp](src/DspChain.cpp).

The DSP chain currently applies these stages in order:

1. Delay-line based pitch shifting using two moving read heads.
2. `tanh()` saturation for heavier harmonic content.
3. Wave folding around a configurable threshold.
4. A dark one-pole smoothing filter.
5. A final gain stage and clamp to keep the output controlled.

WAV helpers in [src/WavHandler.cpp](src/WavHandler.cpp) support uncompressed 16-bit PCM files for offline load/save workflows.

## Requirements

- A C++17 compiler such as `clang++`.
- PortAudio headers and library installed on your system.
- macOS microphone permission enabled for the terminal or editor running the binary.

If you use Homebrew on Apple Silicon, PortAudio is typically available under `/opt/homebrew`.

## Build

This repository does not currently include a build system, so you can compile the sources directly:

```bash
clang++ -std=c++17 \
	src/main.cpp src/DspChain.cpp src/WavHandler.cpp \
	-I/opt/homebrew/include \
	-L/opt/homebrew/lib \
	-lportaudio \
	-o batfleck_live
```

If your PortAudio install lives elsewhere, adjust the include and library paths accordingly.

## Run

```bash
./batfleck_live
```

After launch, the program opens the default microphone stream, begins processing in real time, and waits for Enter to stop.

## Tuning

The live settings are initialized in [src/main.cpp](src/main.cpp):

- `pitchRatio` controls the pitch shift amount.
- `foldThreshold` controls how aggressively the wave folder distorts the signal.
- `sampleRate` and buffer size control latency and sound character.

To change the sound, edit those values and rebuild.

## Project Layout

- [src/main.cpp](src/main.cpp): PortAudio entry point and live callback.
- [src/DspChain.cpp](src/DspChain.cpp): Core voice modulation pipeline.
- [src/DspChain.hpp](src/DspChain.hpp): DSP class interface.
- [src/WavHandler.cpp](src/WavHandler.cpp): WAV load/save helpers.
- [src/WavHandler.hpp](src/WavHandler.hpp): WAV data structures and declarations.

## Notes

- The current implementation expects mono input/output.
- WAV support is limited to uncompressed 16-bit PCM.
- The output is intentionally aggressive and designed for a stylized modulated voice rather than transparent speech preservation.