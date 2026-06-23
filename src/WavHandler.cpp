#include <iostream>
#include <fstream> // to handle .wav audio
#include <vector>
#include <cstdint>
#include <algorithm>

#include <string>
#include <stdexcept>

#include "WavHandler.hpp"

using namespace std;

// this is the most complex part
// reading the .wav file is described in this 44 byte header
struct WAVHeader {
    char chunk_ID[4];
    uint32_t chunk_size;
    char format[4];

    char sub_chunk1_ID[4];
    uint32_t sub_chunk1_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bytes_per_sample;

    char sub_chunk2_ID[4];
    uint32_t sub_chunk2_size;

    // this makes a total of 44 bytes
};

// in stream, we process the input
vector<float> loadWavPayload(const string& filename, WAVHeader& header){

    // we open the file in strict binary mode
    ifstream file(filename, ios::binary);
    if (!file){
        throw runtime_error("Could not open file " + filename);
    }

    // what is reinterpret_cast
    file.read(reinterpret_cast<char*>(&header), sizeof(WAVHeader));

    if (header.audio_format != 1 || header.bytes_per_sample != 16){
        throw runtime_error("Error the engine requires uncompressed 16-bit PCM WAV file");
    }

    // sub_chunk2_size = total bytes of audio data
    // Since it's 16-bit audio (2 bytes per sample), we divide by 2 to get the total number of samples.
    uint32_t numSamples = header.sub_chunk2_size / 2;

    vector<int16_t> rawIntBuffer(numSamples);
    file.read(reinterpret_cast<char*>(rawIntBuffer.data()), header.sub_chunk2_size);

    // The DSP Bridge: Normalize integers to -1.0 to 1.0 floats
    vector<float> dspBuffer(numSamples);
    for (size_t i = 0; i < numSamples; ++i){
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

int main(){
    WAVHeader inHeader;
    vector<float> audioData;
    vector<float> processedData;

    try{
        audioData = loadWavPayload("../assets/test1.wav", inHeader);

        cout << "WAV loaded successfully" << endl;
        cout << "Sample Rate: " << inHeader.sample_rate << "Hz" << endl;
        cout << "Total Samples: " << audioData.size() << endl;
    }
    catch (const exception& e){
        cerr << e.what() << endl;
        return 1;
    }

    const size_t BUFFER_SIZE = 512;
    processedData.reserve(audioData.size());

    cout << "Starting DSP Pipeline...\n";

    for (size_t i = 0; i < audioData.size(); i += BUFFER_SIZE){
        size_t currentChunkSize = min(BUFFER_SIZE, audioData.size() - i);

        vector<float> chunk(audioData.begin() + i , audioData.begin() + i + currentChunkSize);

        // our DSP math will be here
        ////

        processedData.insert(processedData.end(), chunk.begin(), chunk.end());
    }

    cout << "DSP Processing Complete.\n";

    // save the rendered output
    try {
        saveWavPayload("batman_output.wav", inHeader, processedData);
        cout << "[+] Saved transformed voice to: batman_output.wav\n";
    } catch (const exception& e) {
        cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}