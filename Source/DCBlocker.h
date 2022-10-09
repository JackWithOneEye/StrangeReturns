#pragma once

#include <JuceHeader.h>

class DCBlocker
{
public:
    DCBlocker() {}
    ~DCBlocker() {}
    
    float processSample(float x)
    {
        auto y = x - xn1 + coeff * yn1;
        xn1 = x;
        yn1 = y;
        return y;
    }
    
    void reset(float sampleRate)
    {
        fs = sampleRate;
        xn1 = 0.0f;
        yn1 = 0.0f;
    }
    
private:
    static constexpr float coeff = 0.995f;

    float fs = 44100.0f;
    
    float xn1 = 0.0f;
    float yn1 = 0.0f;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DCBlocker)
};
