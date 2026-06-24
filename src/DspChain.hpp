#pragma once
#include <vector>

class DspChain {
private:
    float m_phase;
    float m_sampleRate;

    // pitch shifting
    std::vector<float> m_delayBuffer;
    int m_writeIdx;
    float m_readIdxA;
    float m_readIdxB;
    int m_windowSize;

public:
    DspChain(float sampleRate);
    void processVoice(std::vector<float>& chunk, float ringModFreq, float foldThreshold, float pitchShiftRatio);
};