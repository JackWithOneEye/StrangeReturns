#pragma once

#include <JuceHeader.h>

class NoiseGenerator
{
public:
    NoiseGenerator() {}
    virtual ~NoiseGenerator() {}
    
    void reset(float sampleRate)
    {
        fs = sampleRate;
    }
    
    virtual float nextValue() { return 0.0f; };
    
protected:
    float fs = 44100.0f;
    
    Random random;
    
    inline float uniformRandomValue()
    {
        return 2.0f * random.nextFloat() - 1.0f;
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoiseGenerator)
};

class WhiteNoiseGenerator : NoiseGenerator
{
public:
    WhiteNoiseGenerator() : NoiseGenerator() {}
    
    void reset(float sampleRate)
    {
        NoiseGenerator::reset(sampleRate);
    }
    
    float nextValue() override
    {
        return uniformRandomValue();
    }
    
private:
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WhiteNoiseGenerator)
};

class BrownianNoiseGenerator : NoiseGenerator
{
public:
    BrownianNoiseGenerator() : NoiseGenerator(), normalisedSamples(NUM_BUFFERED_SAMPLES) {}
    
    void reset(float sampleRate)
    {
        NoiseGenerator::reset(sampleRate);
        lastUnnormalisedSample = 0.0f;
        // currPosition = 0;
        generateNormalisedSamples();
    }
    
    float nextValue() override
    {
        if (currPosition == NUM_BUFFERED_SAMPLES)
            generateNormalisedSamples();
        
        return normalisedSamples[currPosition++];
    }
    
private:
    static constexpr int NUM_BUFFERED_SAMPLES = 44100;
    
    float lastUnnormalisedSample = 0.0f;
    std::vector<float> normalisedSamples;
    int currPosition = 0;
    
    void generateNormalisedSamples();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BrownianNoiseGenerator)
};
