#include <iostream>
#include <vector>
#include <portaudio.h> // will have compile with header location flags
#include "DspChain.hpp"

using namespace std;

// this is the dsp engine and settings that will 
// be passed in the portaudio function calls
struct AudioContext{
    DspChain* dspEngine;
    float pitchRatio;
    float foldThreshold;
};

// must strictly match the signature portaudio expects
// this is the callback function of the frames that will be continuosly passed
// used by the mic audio driver
static int frameCallback(
    const void *inputBuffer, 
    void *outputBuffer,
    unsigned long framesPerBuffer, 
    const PaStreamCallbackTimeInfo* timeInfo, 
    PaStreamCallbackFlags statusFlags, 
    void *userData)
{
    float *in = (float*)inputBuffer;
    float *out = (float*)outputBuffer;
    AudioContext *ctx = (AudioContext*)userData;

    // if the mic went silent or disconnected, output silence (every frame multiplies with zero)
    if (in == nullptr) {
        for (unsigned int i = 0; i < framesPerBuffer; i++) out[i] = 0.0f;
        return paContinue;
    }

    vector<float> chunk(in, in + framesPerBuffer);

    // our dsp math
    ctx->dspEngine->processVoice(chunk, ctx->foldThreshold, ctx->pitchRatio);

    for (unsigned int i = 0; i < framesPerBuffer; i++){
        out[i] = chunk[i];
    }

    return paContinue;
}

int main(){
    cout << "Starting Portaudio engine...\n";

    PaError err = Pa_Initialize();
    if (err != paNoError){
        cerr << "PortAudio Error: " << Pa_GetErrorText(err) << "/n";
        return 1;
    }

    float sampleRate = 44100.0f;
    DspChain engine(sampleRate);

    AudioContext context;
    context.dspEngine = &engine;
    context.foldThreshold = 0.85f;
    context.pitchRatio = 0.72f;

    // open the live audio stream
    PaStream *stream;
    err = Pa_OpenDefaultStream(&stream, 1, 1, paFloat32, sampleRate, 512, frameCallback, &context);

    if (err != paNoError) {
        cerr << "Stream Open Error: " << Pa_GetErrorText(err) << "\n";
        return 1;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        cerr << "Stream Start Error: " << Pa_GetErrorText(err) << "\n";
        return 1;
    }

    cout << "Starting live voice modulation...Press Enter to stop\n";

    cin.get();

    err = Pa_CloseStream(stream);
    Pa_Terminate();
    cout << "Voice modulation ended.\n";

    return 0;
}