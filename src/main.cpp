#include <iostream>
#include <vector>
#include <algorithm>
#include "WavHandler.hpp"
#include "DspChain.hpp"

using namespace std;

int main() {
    WAVHeader header;
    vector<float> audioData;
    vector<float> processedData; // this will hold the output voice

    try{
        audioData = loadWavPayload("../assets/test2.wav", header);
        cout << "WAV File loaded. Total Samples: " << audioData.size() + "\n";
    }
    catch (const exception& e){
        cerr << e.what() << endl;
        return 1;
    }

    const size_t BUFFER_SIZE = 512; // for the fake live DSP Loop

    // we already know how big the output file could be so
    // reserve the ram upfront to make reading loop lightning fast
    processedData.reserve(audioData.size());

    cout << "Starting DSP Pipeline...\n";

    DspChain dspEngine(static_cast<float>(header.sample_rate));

    for (size_t i = 0; i < audioData.size(); i += BUFFER_SIZE){
        // determine the chunk size
        size_t currentChunkSize = min(BUFFER_SIZE, audioData.size() - i);

        // create a temprory chunk but in live system this chunk is handed to us directly by the mac mic
        vector<float> chunk(audioData.begin() + i, audioData.begin() + i + currentChunkSize);

        dspEngine.processVoice(chunk, 35.0f, 0.7f, 0.75f);

        processedData.insert(processedData.end(), chunk.begin(), chunk.end());
    }

    cout << "DSP Processing complete\n";

    try {
        saveWavPayload("batman_output.wav", header, processedData);
        std::cout << "[+] Saved transformed voice to: batman_output.wav\n";
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}