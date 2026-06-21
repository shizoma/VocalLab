#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
VocalLabEditor::VocalLabEditor (VocalLabProcessor& p)
    : AudioProcessorEditor (&p), proc (p),
      knobIntensity ("INTENSITY", p.apvts, "macro_intensity"),
      knobColor     ("COLOR",     p.apvts, "macro_color"),
      knobSpace     ("SPACE",     p.apvts, "macro_space"),
      knobTune      ("TUNE",      p.apvts, "macro_tune"),
      knobOutGain   ("OUT GAIN",  p.apvts, "out_gain"),
      meterIn  (p, LevelMeter::Input),
      meterOut (p, LevelMeter::Output),
      grMeter (p),
      knobGateThresh("GATE THR",  p.apvts, "gate_thresh"),
      knobCompThresh("COMP THR",  p.apvts, "comp_thresh"),
      knobCompRatio ("RATIO",     p.apvts, "comp_ratio"),
      knobSatDrive  ("DRIVE",     p.apvts, "sat_drive"),
      knobAirGain   ("AIR GAIN",  p.apvts, "air_gain"),
      knobChorusMix ("CHORUS",    p.apvts, "chorus_mix"),
      knobDelayMix  ("DELAY",     p.apvts, "delay_mix"),
      knobReverbWet ("REV WET",   p.apvts, "reverb_wet"),
      knobReverbSize("REV SIZE",  p.apvts, "reverb_size")
{
    setLookAndFeel (&laf);

    // Preset box
    buildPresetBox();
    addAndMakeVisible (presetBox);

    // Bypass button
    bypassBtn.setClickingTogglesState (true);
    bypassAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        p.apvts, "global_bypass", bypassBtn);
    addAndMakeVisible (bypassBtn);

    // Advanced button
    advancedBtn.setClickingTogglesState (true);
    advancedBtn.onClick = [this]
    {
        updateAdvancedModeState();
    };
    advancedAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        p.apvts, "advanced_mode", advancedBtn);
    addAndMakeVisible (advancedBtn);

    // Knobs
    addAndMakeVisible (knobIntensity);
    addAndMakeVisible (knobColor);
    addAndMakeVisible (knobSpace);
    addAndMakeVisible (knobTune);
    addAndMakeVisible (knobOutGain);

    // Advanced Knobs
    addAndMakeVisible (knobGateThresh);
    addAndMakeVisible (knobCompThresh);
    addAndMakeVisible (knobCompRatio);
    addAndMakeVisible (knobSatDrive);
    addAndMakeVisible (knobAirGain);
    addAndMakeVisible (knobChorusMix);
    addAndMakeVisible (knobDelayMix);
    addAndMakeVisible (knobReverbWet);
    addAndMakeVisible (knobReverbSize);

    // Meters
    addAndMakeVisible (meterIn);
    addAndMakeVisible (meterOut);
    addAndMakeVisible (grMeter);

    // Initialize state
    bool isAdv = p.apvts.getRawParameterValue("advanced_mode")->load() > 0.5f;
    advancedBtn.setToggleState(isAdv, juce::dontSendNotification);
    updateAdvancedModeState();

    startTimerHz (30);
}

//==============================================================================
void VocalLabEditor::updateAdvancedModeState()
{
    bool isAdv = advancedBtn.getToggleState();
    setSize (700, isAdv ? 650 : 400);

    // Dim and disable macro knobs if advanced is ON
    float alpha = isAdv ? 0.3f : 1.0f;
    knobIntensity.setAlpha (alpha);
    knobColor.setAlpha     (alpha);
    knobSpace.setAlpha     (alpha);
    knobTune.setAlpha      (alpha);

    knobIntensity.setInterceptsMouseClicks (!isAdv, !isAdv);
    knobColor.setInterceptsMouseClicks     (!isAdv, !isAdv);
    knobSpace.setInterceptsMouseClicks     (!isAdv, !isAdv);
    knobTune.setInterceptsMouseClicks      (!isAdv, !isAdv);
}

//==============================================================================
void VocalLabEditor::buildPresetBox()
{
    for (int i = 0; i < kNumArtistPresets; ++i)
        presetBox.addItem (kArtistPresets[i].name, i + 1);

    presetBox.setSelectedId (1, juce::dontSendNotification);
    presetBox.onChange = [this]
    {
        const int idx = presetBox.getSelectedId() - 1;
        proc.loadArtistPreset (idx);
    };
}

//==============================================================================
void VocalLabEditor::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const int W = getWidth();
    const int H = getHeight();
    const bool isAdv = advancedBtn.getToggleState();

    // ── Background ───────────────────────────────────────────
    g.setColour (juce::Colour (VocalLabLAF::ColBg));
    g.fillAll();

    // ── Top header card ──────────────────────────────────────
    g.setColour (juce::Colour (VocalLabLAF::ColPanel));
    g.fillRoundedRectangle (bounds.withHeight (70), 0.0f);

    // ── Accent line ──────────────────────────────────────────
    g.setColour (juce::Colour (VocalLabLAF::ColAccent).withAlpha (0.6f));
    g.fillRect (0.0f, 69.0f, bounds.getWidth(), 1.5f);

    // ── Logo / Title ─────────────────────────────────────────
    {
        juce::Font titleFont = juce::Font (juce::FontOptions (28.0f).withStyle ("Bold"));
        g.setFont (titleFont);

        // Glow effect
        g.setColour (juce::Colour (VocalLabLAF::ColAccent).withAlpha (0.18f));
        g.drawText ("VOCAL LAB", 16, 16, 220, 38, juce::Justification::centredLeft);
        g.setColour (juce::Colour (VocalLabLAF::ColAccent).withAlpha (0.18f));
        g.drawText ("VOCAL LAB", 18, 18, 220, 38, juce::Justification::centredLeft);

        g.setColour (juce::Colour (VocalLabLAF::ColText));
        g.drawText ("VOCAL LAB", 17, 17, 220, 38, juce::Justification::centredLeft);

        // Subtitle
        g.setFont (juce::Font (juce::FontOptions (11.0f)));
        g.setColour (juce::Colour (VocalLabLAF::ColAccent));
        g.drawText ("AI VOCAL CHAIN", 19, 47, 200, 14, juce::Justification::centredLeft);
    }

    // ── Section labels ───────────────────────────────────────
    g.setFont (juce::Font (juce::FontOptions (10.5f).withStyle ("Bold")));
    g.setColour (juce::Colour (VocalLabLAF::ColTextDim));
    g.drawText ("ARTIST PRESET",    18,      82, 120, 16, juce::Justification::centredLeft);
    g.drawText ("IN",               W - 130, 82, 24,  16, juce::Justification::centred);
    g.drawText ("OUT",              W - 100, 82, 28,  16, juce::Justification::centred);
    g.drawText ("GR",               W - 64,  82, 24,  16, juce::Justification::centred);

    // ── Knob section card (Macros) ───────────────────────────
    g.setColour (juce::Colour (VocalLabLAF::ColCard));
    g.fillRoundedRectangle (10.0f, 150.0f, bounds.getWidth() - 20.0f, 228.0f, 10.0f);
    g.setColour (juce::Colour (VocalLabLAF::ColKnobRim).withAlpha (0.5f));
    g.drawRoundedRectangle (10.0f, 150.0f, bounds.getWidth() - 20.0f, 228.0f, 10.0f, 1.0f);

    // ── Separator between macro knobs and out gain ────────────
    g.setColour (juce::Colour (VocalLabLAF::ColKnobRim).withAlpha (0.35f));
    const int sepX = W - 115;
    g.fillRect (sepX, 158, 1, 212);

    // ── Advanced Panel ───────────────────────────────────────
    if (isAdv)
    {
        g.setColour (juce::Colour (VocalLabLAF::ColCard));
        g.fillRoundedRectangle (10.0f, 390.0f, bounds.getWidth() - 20.0f, 230.0f, 10.0f);
        g.setColour (juce::Colour (VocalLabLAF::ColAccent).withAlpha (0.4f));
        g.drawRoundedRectangle (10.0f, 390.0f, bounds.getWidth() - 20.0f, 230.0f, 10.0f, 1.0f);
        
        g.setFont (juce::Font (juce::FontOptions (10.5f).withStyle ("Bold")));
        g.setColour (juce::Colour (VocalLabLAF::ColAccent));
        g.drawText ("ADVANCED SETTINGS", 20, 396, 200, 16, juce::Justification::centredLeft);
    }

    // ── Copyright ────────────────────────────────────────────
    g.setFont (juce::Font (juce::FontOptions (9.5f)));
    g.setColour (juce::Colour (VocalLabLAF::ColTextDim).withAlpha (0.5f));
    g.drawText (juce::CharPointer_UTF8 ("\xc2\xa9 CiptaSuara & Soba Studio"),
                10, H - 18, W - 20, 14, juce::Justification::centred);

    // ── Version ──────────────────────────────────────────────
    g.drawText ("v1.0.0", W - 56, H - 18, 48, 14, juce::Justification::centredRight);
}

//==============================================================================
void VocalLabEditor::resized()
{
    const int W = getWidth();

    // Header zone
    presetBox.setBounds (18, 100, 260, 38);
    advancedBtn.setBounds (W - 328, 20, 90, 30);
    bypassBtn.setBounds (W - 228, 20, 90, 30);

    // I/O meters + GR meter — top-right
    meterIn .setBounds (W - 130, 100, 26, 38);
    meterOut.setBounds (W -  98, 100, 26, 38);
    grMeter .setBounds (W -  62, 100, 46, 38);

    // Macro Knob section
    const int knobY  = 158;
    const int knobH  = 204;
    const int availW = W - 20 - 110; 
    const int knobW  = availW / 4;

    knobIntensity.setBounds (10           + 10, knobY, knobW - 4, knobH);
    knobColor.setBounds     (10 + knobW   + 10, knobY, knobW - 4, knobH);
    knobSpace.setBounds     (10 + knobW*2 + 10, knobY, knobW - 4, knobH);
    knobTune.setBounds      (10 + knobW*3 + 10, knobY, knobW - 4, knobH);

    // Out gain
    knobOutGain.setBounds (W - 108, knobY, 96, knobH);

    // Advanced Knobs
    const bool isAdv = advancedBtn.getToggleState();
    if (isAdv)
    {
        const int advY = 416;
        const int advH = 190;
        const int advCols = 9;
        const int advW = (W - 40) / advCols;

        knobGateThresh.setBounds (20 + advW*0, advY, advW, advH);
        knobCompThresh.setBounds (20 + advW*1, advY, advW, advH);
        knobCompRatio.setBounds  (20 + advW*2, advY, advW, advH);
        knobSatDrive.setBounds   (20 + advW*3, advY, advW, advH);
        knobAirGain.setBounds    (20 + advW*4, advY, advW, advH);
        knobChorusMix.setBounds  (20 + advW*5, advY, advW, advH);
        knobDelayMix.setBounds   (20 + advW*6, advY, advW, advH);
        knobReverbWet.setBounds  (20 + advW*7, advY, advW, advH);
        knobReverbSize.setBounds (20 + advW*8, advY, advW, advH);

        knobGateThresh.setVisible(true);
        knobCompThresh.setVisible(true);
        knobCompRatio.setVisible(true);
        knobSatDrive.setVisible(true);
        knobAirGain.setVisible(true);
        knobChorusMix.setVisible(true);
        knobDelayMix.setVisible(true);
        knobReverbWet.setVisible(true);
        knobReverbSize.setVisible(true);
    }
    else
    {
        knobGateThresh.setVisible(false);
        knobCompThresh.setVisible(false);
        knobCompRatio.setVisible(false);
        knobSatDrive.setVisible(false);
        knobAirGain.setVisible(false);
        knobChorusMix.setVisible(false);
        knobDelayMix.setVisible(false);
        knobReverbWet.setVisible(false);
        knobReverbSize.setVisible(false);
    }
}
