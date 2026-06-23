#pragma once
#include <vector>

class DspChain {
private:
    float m_phase;
    float m_sampleRate;

public:
    DspChain(float sampleRate) : m_phase(0.0f), m_sampleRate(sampleRate) {}

    void processVoice(std::vector<float>& chunk, float ringModFreq, float foldThreshold);
};