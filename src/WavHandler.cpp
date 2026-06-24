#include <iostream>
#include <fstream> // to handle .wav audio
#include <vector>
#include <cstdint>
#include <cstring>
#include <algorithm>

#include <string>
#include <stdexcept>

#include "WavHandler.hpp"

using namespace std;

// this is the most complex part
// reading the .wav file is described in this 44 byte header

// in stream, we process the input
vector<float> loadWavPayload(const string& filename, WAVHeader& header) {
    ifstream file(filename, ios::binary);
    if (!file) {
        throw runtime_error("FATAL: Could not open file " + filename);
    }

    // 1. Read the 36-byte format baseline
    file.read(reinterpret_cast<char*>(&header), 36);

    if (header.audio_format != 1 || header.bytes_per_sample != 16) {
        throw runtime_error("ERROR: Engine requires uncompressed 16-bit PCM WAV.");
    }

    // 2. The Chunk Hunter (Patched for Byte Alignment)
    char currentChunkID[4];
    uint32_t currentChunkSize;
    bool foundData = false;

    while (file.read(currentChunkID, 4)) {
        file.read(reinterpret_cast<char*>(&currentChunkSize), 4);
        
        if (strncmp(currentChunkID, "data", 4) == 0) {
            foundData = true;
            header.sub_chunk2_ID[0] = 'd';
            header.sub_chunk2_ID[1] = 'a';
            header.sub_chunk2_ID[2] = 't';
            header.sub_chunk2_ID[3] = 'a';
            header.sub_chunk2_size = currentChunkSize;
            break;
        } else {
            // SECURITY PATCH: Force word-alignment to prevent memory offset corruption
            uint32_t bytesToSkip = currentChunkSize;
            if (bytesToSkip % 2 != 0) {
                bytesToSkip += 1; 
            }
            file.seekg(bytesToSkip, ios::cur);
        }
    }

    if (!foundData) {
        throw runtime_error("FATAL: Could not locate the 'data' chunk.");
    }

    // 3. Safe Memory Allocation
    uint32_t numSamples = header.sub_chunk2_size / 2;
    vector<int16_t> rawIntBuffer(numSamples);
    
    // SECURITY PATCH: Read exactly what we allocated, no more.
    file.read(reinterpret_cast<char*>(rawIntBuffer.data()), numSamples * 2);

    // 4. Floating Point Conversion
    vector<float> dspBuffer(numSamples);
    for (size_t i = 0; i < numSamples; ++i) {
        dspBuffer[i] = static_cast<float>(rawIntBuffer[i]) / 32768.0f;
    }

    return dspBuffer;
}

// out stream, we process the output
void saveWavPayload(const string& filename, const WAVHeader& header, const vector<float>& dspBuffer){

    ofstream file(filename, ios::binary);
    if (!file){
        throw runtime_error("Could not create output file! " + filename);
    }

    WAVHeader outHeader = header;

    uint32_t numSamples = dspBuffer.size();
    outHeader.sub_chunk2_size = numSamples * 2;
    outHeader.chunk_size = 36 + outHeader.sub_chunk2_size;

    file.write(reinterpret_cast<const char*>(&outHeader), sizeof(WAVHeader));

    vector<int16_t> intBuffer(numSamples);

    // i think std::clam will have to compiled with another version of c++
    for (size_t i = 0; i < numSamples; ++i){
        float clampedSample = clamp(dspBuffer[i], -1.0f, 1.0f);
        intBuffer[i] = static_cast<int16_t>(clampedSample * 32767.0f);
    }
    
    file.write(reinterpret_cast<const char*>(intBuffer.data()), outHeader.sub_chunk2_size);
}