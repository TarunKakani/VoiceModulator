#include "DspChain.hpp"
#include <cmath>
#include <algorithm>

using namespace std;

// ??
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

DspChain::DspChain(float sampleRate) : m_sampleRate(sampleRate), m_phase(0.0f) {
    // A 2048-sample window is about 46ms of audio. Good for deep voices.
    m_windowSize = 2048; 
    
    // Fill the delay buffer with absolute silence (0.0f) to start
    m_delayBuffer.resize(m_windowSize, 0.0f);
    
    // Initialize our pointers
    m_writeIdx = 0;
    m_readIdxA = 0.0f;
    // Set Reader B exactly halfway across the buffer from Reader A
    m_readIdxB = static_cast<float>(m_windowSize / 2); 
}

void DspChain::processVoice(vector<float>& chunk, float ringModFreq, float foldThreshold, float pitchShiftRatio){
    
    // 
    float phaseIncrement = 2.0f * M_PI * ringModFreq / m_sampleRate;

    for (size_t n = 0; n < chunk.size(); ++n){
        float inputSample = chunk[n];

        // pitch shifting
        m_delayBuffer[m_writeIdx] = inputSample;

        // 2. Read the delayed samples from our two separate pointers
        // We cast to int for basic array indexing (ignoring sub-sample interpolation for simplicity here)
        float sampleA = m_delayBuffer[static_cast<int>(m_readIdxA)];
        float sampleB = m_delayBuffer[static_cast<int>(m_readIdxB)];

        // 3. Calculate the Crossfade Windows (The Math Magic)
        // We use a basic sine wave to smoothly fade the volume of A and B up and down
        // depending on where they are inside the buffer.
        float windowA = std::sin(M_PI * (m_readIdxA / m_windowSize));
        float windowB = std::sin(M_PI * (m_readIdxB / m_windowSize));

        // 4. Multiply the samples by their window volumes and add them together!
        float pitchedSample = (sampleA * windowA) + (sampleB * windowB);

        // 5. Advance the pointers
        // The Write pointer moves at normal speed (+1)
        m_writeIdx = (m_writeIdx + 1) % m_windowSize;
        
        // The Read pointers move slower based on the pitch ratio (e.g., +0.8)
        m_readIdxA += pitchShiftRatio;
        if (m_readIdxA >= m_windowSize) m_readIdxA -= m_windowSize;

        m_readIdxB += pitchShiftRatio;
        if (m_readIdxB >= m_windowSize) m_readIdxB -= m_windowSize;

        // ring modulation
        // generate the carrier sine wave sample using our persistent class phase
        float carrier = sin(m_phase);

        // multiply the audio sample by the carrier
        float modulatedSample = pitchedSample * carrier;

        // advance the phase for the next sample
        m_phase += phaseIncrement;

        // keep the phase bounded b/w 0 and 2*PI maintain floating-point accuracy
        if (m_phase >= 2.0f * M_PI){
            m_phase -= 2.0f * M_PI;
        }

        if (modulatedSample > foldThreshold) {
            modulatedSample = foldThreshold - (modulatedSample - foldThreshold);
        } else if (modulatedSample < -foldThreshold) {
            modulatedSample = -foldThreshold - (modulatedSample + foldThreshold);
        }

        // wave folding
        // check for positive threshold crossing
        if (inputSample > foldThreshold){
            float excess = inputSample - foldThreshold; 
            inputSample = foldThreshold - excess;
        }
        else if (inputSample < -foldThreshold) {
            float excess = inputSample + foldThreshold;
            inputSample = -foldThreshold - excess;
        }

        // hard safety clamping
        // ensure that extreme folding values never breach the digital soundcard ceiling
        chunk[n] = clamp(modulatedSample, -0.95f, 0.95f);
    }
}