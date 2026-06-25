#include "DspChain.hpp"
#include <cmath>
#include <algorithm>

using namespace std;

// Define PI if the math library didn't already
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

DspChain::DspChain(float sampleRate) : m_sampleRate(sampleRate), m_phase(0.0f), m_prevFilterSample(0.0f) {
    // A 2048-sample window is about 46ms of audio. Good for deep voices.
    m_windowSize = 4096; 
    
    // Fill the delay buffer with absolute silence (0.0f) to start
    m_delayBuffer.resize(m_windowSize, 0.0f);
    
    // Initialize our pointers
    m_writeIdx = 0;
    m_readIdxA = 0.0f;
    // Set Reader B exactly halfway across the buffer from Reader A
    m_readIdxB = static_cast<float>(m_windowSize / 2); 
}

void DspChain::processVoice(vector<float>& chunk, float foldThreshold, float pitchShiftRatio){
    
    // We want the filter to be darker now to cut the remaining treble. 
    // 0.15f is very dark and muffled (heavy mask effect).
    float filterAlpha = 0.15f;

    for (size_t n = 0; n < chunk.size(); ++n){
        float inputSample = chunk[n];

        // ----------------------------------------------------
        // STAGE 1: PITCH SHIFTING (Now with deeper bass retention)
        // ----------------------------------------------------
        m_delayBuffer[m_writeIdx] = inputSample;

        float sampleA = m_delayBuffer[static_cast<int>(m_readIdxA)];
        float sampleB = m_delayBuffer[static_cast<int>(m_readIdxB)];

        float windowA = std::sin(M_PI * (m_readIdxA / m_windowSize));
        float windowB = std::sin(M_PI * (m_readIdxB / m_windowSize));

        float pitchedSample = (sampleA * windowA) + (sampleB * windowB);

        m_writeIdx = (m_writeIdx + 1) % m_windowSize;
        
        m_readIdxA += pitchShiftRatio;
        if (m_readIdxA >= m_windowSize) m_readIdxA -= m_windowSize;

        m_readIdxB += pitchShiftRatio;
        if (m_readIdxB >= m_windowSize) m_readIdxB -= m_windowSize;

        // ----------------------------------------------------
        // STAGE 2: ANALOG SATURATION (The Cinematic Secret)
        // ----------------------------------------------------
        // We multiply the volume by 4.0 (Overdrive) and push it through a tanh curve.
        // This makes the voice massive, thick, and brutally loud without screeching.
        float mechanicalSample = tanh(pitchedSample * 4.0f);

        // Keep a little bit of the wave folder just for a slight metallic edge
        if (mechanicalSample > foldThreshold){
            float excess = mechanicalSample - foldThreshold; 
            mechanicalSample = foldThreshold - excess;
        }
        else if (mechanicalSample < -foldThreshold) {
            float excess = mechanicalSample + foldThreshold;
            mechanicalSample = -foldThreshold - excess;
        }
    
        // ----------------------------------------------------
        // STAGE 3: MIXING, FILTERING & MASTER LIMITER
        // ----------------------------------------------------
        
        // 50/50 mix of the deep voice and the overdriven voice
        float mixedSample = (pitchedSample * 0.5f) + (mechanicalSample * 0.5f);

        // Muffle it with the heavy low-pass filter
        float smoothedSample = (mixedSample * filterAlpha) + (m_prevFilterSample * (1.0f - filterAlpha));
        m_prevFilterSample = smoothedSample;

        // THE WALL OF SOUND: Multiply the final smoothed audio by a massive gain boost (2.5x).
        // The clamp immediately chops the top off, creating a thick, booming "brickwall" volume.
        float finalOutput = smoothedSample * 2.5f;

        chunk[n] = clamp(finalOutput, -0.95f, 0.95f);
    }
}