#include "DelayProcessor.h"

inline float DelayProcessor::applyBitCrusher(float x, float depth)
{
    if (depth > MIN_BITCRUSHER_Q)
        return depth * ((int)(x / depth));
    
    return x;
}

inline float DelayProcessor::applyBitMod(float dry, float wet, float wetWithFx, float level)
{
    if (bmOperation == BitModulation::Operation::NONE)
        return wetWithFx;
    
    auto operand1 = 0.0f;
    auto operand2 = 0.0f;
    
    if (bmOperands == BitModOperands::POST_FX_POST_FX)
    {
        operand1 = operand2 = wetWithFx;
    }
    else if (bmOperands == BitModOperands::PRE_FX_POST_FX)
    {
        operand1 = wet;
        operand2 = wetWithFx;
    }
    else if (bmOperands == BitModOperands::DRY_POST_FX)
    {
        operand1 = dry;
        operand2 = wetWithFx;
    }
    
    return bitModOpFunc(operand1, operand2 * level);
}

inline float DelayProcessor::applyDecimator(float x, float reduction, float phasorShift, int channel)
{
    decimPhasor[channel] += reduction;
    if (decimPhasor[channel] + phasorShift >= 1.0f)
    {
        decimPhasor[channel] -= 1.0f;
        decimCurrentOutput[channel] = x;
    }
    return decimCurrentOutput[channel];
}

inline float DelayProcessor::applyEffects(float xDry, float xWet, float phaseFlipSmoothed, float bcDepth, float decimReduction, float decimStereoSpread, float bmLevel, int channel)
{
    auto y = xWet;
    
    // phase
    y *= phaseFlipSmoothed;
    
    // bit crusher
    if (bcDepth > MIN_BITCRUSHER_Q)
        y = bcDepth * ((int)(y / bcDepth));
    
    // decimator
    decimPhasor[channel] += decimReduction;
    auto stereoPhaseShift = channel == 0 ? 0.0f : decimStereoSpread;
    if (decimPhasor[channel] + stereoPhaseShift >= 1.0f)
    {
        decimPhasor[channel] -= 1.0f;
        decimCurrentOutput[channel] = y;
    }
    y = decimCurrentOutput[channel];
    
    // LPF if PRE_BITMOD
    if (lpfPosition == LpfPosition::PRE_BITMOD)
    {
        y = lpf[channel].processSample(y);
    }
    
    // bit modulation
    if (bmOperation != BitModulation::Operation::NONE)
    {
        auto operand1 = 0.0f;
        auto operand2 = 0.0f;
        
        if (bmOperands == BitModOperands::POST_FX_POST_FX)
        {
            operand1 = operand2 = y;
        }
        else if (bmOperands == BitModOperands::PRE_FX_POST_FX)
        {
            operand1 = xWet;
            operand2 = y;
        }
        else if (bmOperands == BitModOperands::DRY_POST_FX)
        {
            operand1 = xDry;
            operand2 = y;
        }
        y = bitModOpFunc(operand1, operand2 * bmLevel);
    }
    y = dcBlocker[channel].processSample(y);
    
    // LPF if POST_BITMOD
    if (lpfPosition == LpfPosition::POST_BITMOD)
    {
        y = lpf[channel].processSample(y);
    }
    
    return y;
}

void DelayProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    fs = (float) sampleRate;
    
    maxModDepth_smpls = MAX_MOD_DEPTH_SECS * fs;
    
    time_smpls.reset(fs, 1.0f);
    feedback_lin.reset(fs, SMOOTHED_VAL_RAMP_LEN_SEC);
    dwMixer.reset();
    dwMixer.prepare({ sampleRate, static_cast<uint32>(samplesPerBlock), 2 });
    dwMixer.setMixingRule(dsp::DryWetMixingRule::balanced);
    
    modRate_Hz.reset(fs, SMOOTHED_VAL_RAMP_LEN_SEC);
    modDepth_lin.reset(fs, SMOOTHED_VAL_RAMP_LEN_SEC);
    
    noiseLevel_lin.reset(fs, SMOOTHED_VAL_RAMP_LEN_SEC);
    
    smoothedPhaseFlip.reset(fs, 0.01f);
    
    bcDepth_lin.reset(fs, SMOOTHED_VAL_RAMP_LEN_SEC);
    
    decimReduction_lin.reset(fs, SMOOTHED_VAL_RAMP_LEN_SEC);
    decimStereoSpread_lin.reset(fs, SMOOTHED_VAL_RAMP_LEN_SEC);
    
    lpfCutoff_Hz.reset(fs, SMOOTHED_VAL_RAMP_LEN_SEC);
    lpfQ_lin.reset(fs, SMOOTHED_VAL_RAMP_LEN_SEC);
    
    bmLevel_lin.reset(fs, SMOOTHED_VAL_RAMP_LEN_SEC);
    
    for (int channel = 0; channel < NUM_CHANNELS; ++channel)
    {
        delayBuffer[channel].createCircularBuffer(((int) fs) * MAX_DELAY_TIME_SEC);
        
        tapeDelayBandpass[channel].reset(fs);
        tapeDelayBandpass[channel].setParameters(725.0f, 0.33f, false, false, 0.0f, 1.0f, 0.0f, 0.0f, false);
        
        delayHiPass[channel].reset(fs);
        delayHiPass[channel].setParameters(100.0f, 0.707f, false, false, 0.0f, 0.0f, 1.0f, 0.0f, false);
        
        modLfo[channel].reset(fs);
        
        decimPhasor[channel] = 0.0f;
        decimCurrentOutput[channel] = 0.0f;
        
        lpf[channel].reset(fs);
        lpf[channel].setParameters(lpfCutoff_Hz.getCurrentValue(), lpfQ_lin.getTargetValue(), false, false, 0.0f, 0.0f, 0.0f, 1.0f, false);
        
        dcBlocker[channel].reset(fs);
    }
    
    whiteNoiseGen.reset(fs);
    brownianNoiseGen.reset(fs);
}

void DelayProcessor::processBlock(AudioBuffer<float> &buffer)
{
    dwMixer.pushDrySamples(buffer);
    const int numChannels = jmin(buffer.getNumChannels(), NUM_CHANNELS);
    const int numSamples = buffer.getNumSamples();
    
    for (int sample = 0; sample < numSamples; ++sample)
    {
        const float delay = time_smpls.getNextValue();
        const float fb = feedback_lin.getNextValue();
        
        bool modValsSmoothing = modRate_Hz.isSmoothing() || modDepth_lin.isSmoothing();
        const float modRate = modRate_Hz.getNextValue();
        const float modDepth = modDepth_lin.getNextValue();
        
        auto noiseLvl = noiseLevel_lin.getNextValue();
        float delayNoise = 0.0f;
        if (noiseLvl > 0.001f)
        {
            if (noiseType == NoiseType::WHITE)
            {
                delayNoise = whiteNoiseGen.nextValue();
            }
            else if (noiseType == NoiseType::BROWNIAN)
            {
                delayNoise = brownianNoiseGen.nextValue();
            }
            else if (noiseType == NoiseType::PINK)
            {
                // TODO
            }
            delayNoise *= noiseLvl;
        }
        
        const float phaseFlipSmoothed = smoothedPhaseFlip.getNextValue();
        
        const float bcDepth = bcDepth_lin.getNextValue();
        
        const float decimReduction = decimReduction_lin.getNextValue();
        const float decimStereoSpread = decimStereoSpread_lin.getNextValue();
        
        bool lpfValsSmoothing = lpfCutoff_Hz.isSmoothing() || lpfQ_lin.isSmoothing();
        const float lpfCutoff = lpfCutoff_Hz.getNextValue();
        const float lpfQ = lpfQ_lin.getNextValue();
        
        const float bmLevel = bmLevel_lin.getNextValue();
        
        for (int channel = 0; channel < numChannels; ++channel)
        {
            float* xPtr = buffer.getWritePointer(channel, sample);
            float x = *xPtr;
            
            if (modValsSmoothing)
            {
                modLfo[channel].setParams(modRate, modDepth, modWave, FastMathLFO::LFOPolarity::UNIPOLAR);
            }
            
            if (lpfValsSmoothing)
            {
                lpf[channel].setParameters(lpfCutoff, lpfQ, false, false, 0.0f, 0.0f, 0.0f, 1.0f, false);
            }
            
            auto modDepthSmpls = modLfo[channel].getNextSample(0.0f) * maxModDepth_smpls;
            auto delayLineOut = delayBuffer[channel].readBuffer(delay + modDepthSmpls);
            delayLineOut += delayNoise;
            
            if (effectsRouting == EffectsRouting::IN)
            {
                delayLineOut = applyEffects(x, delayLineOut, phaseFlipSmoothed, bcDepth, decimReduction, decimStereoSpread, bmLevel, channel);
            }
            
            if (toneType == ToneType::TAPE)
            {
                delayLineOut = softClipper(delayLineOut);
                delayLineOut = TAPE_DEL_LOOP_GAIN * tapeDelayBandpass[channel].processSample(delayLineOut);
            }
            
            auto dlHpfOut = delayHiPass[channel].processSample(delayLineOut);
            
            delayBuffer[channel].writeBuffer(x + fb * dlHpfOut);
            
            if (effectsRouting == EffectsRouting::OUT)
            {
                delayLineOut = applyEffects(x, delayLineOut, phaseFlipSmoothed, bcDepth, decimReduction, decimStereoSpread, bmLevel, channel);
            }
            
            *xPtr = delayLineOut;
        }
    }
    dwMixer.mixWetSamples(buffer);
}
