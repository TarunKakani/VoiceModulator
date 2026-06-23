#include <iostream>
#include <vector>
#include <algorithm>
#include "WavHandler.hpp"

using namespace std;

int main() {
    WAVHeader header;
    vector<float> audioData;
    vector<float> processedData; // this will hold the output voice

    try{
        audioData = loadWavPayload("../assets/test1.wav", header);
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

    for (size_t i = 0; i < audioData.size(); i += BUFFER_SIZE){
        // determine the chunk size
        size_t currentChunkSize = min(BUFFER_SIZE, audioData.size() - i);

        // create a temprory chunk but in live system this chunk is handed to us directly by the mac mic
        vector<float> chunk(audioData.begin() + i, audioData.begin() + i + currentChunkSize);

        // apply the dsp math here
        ////

        processedData.insert(processedData.end(), chunk.begin(), chunk.end());
    }

    cout << "DSP Processing complete\n";
}