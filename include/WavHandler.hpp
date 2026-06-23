#pragma once // this ensures the compiler only loads it once
#include <vector>
#include <string>
#include <cstdint>

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

std::vector<float> loadWavPayload(const std::string& filename, WAVHeader& header);

void saveWavPayload(const std::string& filename, const WAVHeader& header, const std::vector<float>& dspBuffer);