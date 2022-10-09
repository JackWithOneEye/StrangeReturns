#include "NoiseGenerator.h"

void BrownianNoiseGenerator::generateNormalisedSamples()
{
    std::vector<float> unnormalisedSamples(NUM_BUFFERED_SAMPLES);
    unnormalisedSamples[0] = lastUnnormalisedSample + uniformRandomValue();
    
    for (int i = 1; i < NUM_BUFFERED_SAMPLES; i++)
    {
        unnormalisedSamples[i] = 0.95f * unnormalisedSamples[i - 1] + uniformRandomValue(); // leaky integration
    }
    
    auto maxB = *std::max_element(unnormalisedSamples.begin(), unnormalisedSamples.end());
    auto minB = *std::min_element(unnormalisedSamples.begin(), unnormalisedSamples.end());
    
    // normalisedSamples.clear();
    auto minMaxDiff = maxB - minB;
    for (int i = 0; i < NUM_BUFFERED_SAMPLES; i++)
    {
        auto unSmpl = unnormalisedSamples[i];
        normalisedSamples[i] = 0.8f * (2.0f * (unSmpl - minB) / minMaxDiff - 1.0f);
    }
    lastUnnormalisedSample = unnormalisedSamples.back();
    currPosition = 0;
}
