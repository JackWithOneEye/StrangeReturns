#include "PluginProcessor.h"
#include "PluginEditor.h"

StrangeReturnsAudioProcessorEditor::StrangeReturnsAudioProcessorEditor (StrangeReturnsAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    addAllAndMakeVisible(*this, basicControls, modAndNoiseControls, phaseBitCrusherDecimatorControls, lpfControls, bitModControls);
    
    setSize(900, 850);
}

StrangeReturnsAudioProcessorEditor::~StrangeReturnsAudioProcessorEditor()
{
}

//==============================================================================
void StrangeReturnsAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    g.setColour (Colours::white);
    g.setFont (15.0f);
}

void StrangeReturnsAudioProcessorEditor::resized()
{
    const int rowHeight = 150;
    const auto rowWidth = getWidth() * 0.9f;
    const auto rowMarginLeft = getWidth() * 0.025f;
    const int rowGap = 5;
    
    int y = 0;
    
    basicControls.setBounds(rowMarginLeft, y, rowWidth, rowHeight);
    
    y = rowHeight + rowGap;
    modAndNoiseControls.setBounds(rowMarginLeft, y, rowWidth, rowHeight);
    
    y += rowHeight + rowGap;
    phaseBitCrusherDecimatorControls.setBounds(rowMarginLeft, y, rowWidth, rowHeight);
    
    y += rowHeight + rowGap;
    lpfControls.setBounds(rowMarginLeft, y, rowWidth, rowHeight);
    
    y += rowHeight + rowGap;
    bitModControls.setBounds(rowMarginLeft, y, rowWidth, rowHeight);
}
