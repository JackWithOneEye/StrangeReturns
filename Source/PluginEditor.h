#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "EditorComponents.h"

class StrangeReturnsAudioProcessorEditor  : public AudioProcessorEditor
{
public:
    StrangeReturnsAudioProcessorEditor (StrangeReturnsAudioProcessor&);
    ~StrangeReturnsAudioProcessorEditor() override;

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    
    struct BasicControls : public Component
    {
        explicit BasicControls(const StrangeReturnsAudioProcessor::ParameterReferences& state)
            : time(state.time),
            feedback(state.feedback),
            dryWet(state.dryWet),
            toneType(state.toneType),
            effectsRouting(state.effectsRouting)
        {
            addAllAndMakeVisible(*this, time, feedback, dryWet, toneType, effectsRouting);
        }

        void resized() override
        {
            performLayout(getLocalBounds(), time, feedback, dryWet, toneType, effectsRouting);
        }

        AttachedSlider time, feedback, dryWet;
        AttachedCombo toneType, effectsRouting;
    };
    
    struct ModAndNoiseControls : public Component
    {
        explicit ModAndNoiseControls(const StrangeReturnsAudioProcessor::ParameterReferences& state)
            : modRate(state.modRate),
            modDepth(state.modDepth),
            noiseLevel(state.noiseLevel),
            modWave(state.modWave),
            noiseType(state.noiseType)
        {
            addAllAndMakeVisible(*this, modRate, modDepth, modWave, noiseLevel, noiseType);
        }

        void resized() override
        {
            performLayout(getLocalBounds(), modRate, modDepth, modWave, noiseLevel, noiseType);
        }

        AttachedSlider modRate, modDepth, noiseLevel;
        AttachedCombo modWave, noiseType;
    };
    
    struct PhaseBitCrusherDecimatorControls : public Component
    {
        explicit PhaseBitCrusherDecimatorControls(const StrangeReturnsAudioProcessor::ParameterReferences& state)
            : flipPhase(state.flipPhase),
            bcDepth(state.bcDepth),
            decimReduction(state.decimReduction),
            decimStereoSpread(state.decimStereoSpread)
        {
            addAllAndMakeVisible(*this, flipPhase, bcDepth, decimReduction, decimStereoSpread);
        }

        void resized() override
        {
            performLayout(getLocalBounds(), flipPhase, bcDepth, decimReduction, decimStereoSpread);
        }

        AttachedToggle flipPhase;
        AttachedSlider bcDepth, decimReduction, decimStereoSpread;
    };
    
    struct LpfControls : public Component
    {
        explicit LpfControls(const StrangeReturnsAudioProcessor::ParameterReferences& state)
            : lpfCutoff(state.lpfCutoff),
            lpfQ(state.lpfQ),
            lpfPosition(state.lpfPosition)
        {
            addAllAndMakeVisible(*this, lpfCutoff, lpfQ, lpfPosition);
        }

        void resized() override
        {
            performLayout(getLocalBounds(), lpfCutoff, lpfQ, lpfPosition);
        }

        AttachedSlider lpfCutoff, lpfQ;
        AttachedCombo lpfPosition;
    };
    
    struct BitModControls : public Component
    {
        explicit BitModControls(const StrangeReturnsAudioProcessor::ParameterReferences& state)
            : bmLevel(state.bmLevel),
            bmOperation(state.bmOperation),
            bmOperands(state.bmOperands)
        {
            addAllAndMakeVisible(*this, bmLevel, bmOperation, bmOperands);
        }

        void resized() override
        {
            performLayout(getLocalBounds(), bmLevel, bmOperation, bmOperands);
        }

        AttachedSlider bmLevel;
        AttachedCombo bmOperation, bmOperands;
    };
    
    StrangeReturnsAudioProcessor& audioProcessor;
    
    BasicControls basicControls { audioProcessor.getParameterValues() };
    ModAndNoiseControls modAndNoiseControls { audioProcessor.getParameterValues() };
    PhaseBitCrusherDecimatorControls phaseBitCrusherDecimatorControls { audioProcessor.getParameterValues() };
    LpfControls lpfControls { audioProcessor.getParameterValues() };
    BitModControls bitModControls { audioProcessor.getParameterValues() };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StrangeReturnsAudioProcessorEditor)
};
