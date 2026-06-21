#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
VocalLabProcessor::VocalLabProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
#endif
          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
      ),
#else
    : AudioProcessor(),
#endif
      apvts (*this, nullptr, "VocalLabState", createParameterLayout())
{
    // Cache raw parameter pointers for lock-free audio-thread reads
    pIntensity = apvts.getRawParameterValue ("macro_intensity");
    pColor     = apvts.getRawParameterValue ("macro_color");
    pSpace     = apvts.getRawParameterValue ("macro_space");
    pTune      = apvts.getRawParameterValue ("macro_tune");
    pBypass    = apvts.getRawParameterValue ("global_bypass");
    pOutGain   = apvts.getRawParameterValue ("out_gain");

    pAdvanced   = apvts.getRawParameterValue ("advanced_mode");
    pGateThresh = apvts.getRawParameterValue ("gate_thresh");
    pCompThresh = apvts.getRawParameterValue ("comp_thresh");
    pCompRatio  = apvts.getRawParameterValue ("comp_ratio");
    pSatDrive   = apvts.getRawParameterValue ("sat_drive");
    pAirGain    = apvts.getRawParameterValue ("air_gain");
    pChorusMix  = apvts.getRawParameterValue ("chorus_mix");
    pDelayMix   = apvts.getRawParameterValue ("delay_mix");
    pReverbWet  = apvts.getRawParameterValue ("reverb_wet");
    pReverbSize = apvts.getRawParameterValue ("reverb_size");

    // Saturation function: tanh soft-clip (warm analogue-style)
    dspChain.get<kSaturator>().functionToUse = [] (float x)
    {
        return std::tanh (x);
    };
}

VocalLabProcessor::~VocalLabProcessor() {}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout VocalLabProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // ── Macro controls (Beginner Mode) ──────────────────────
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "macro_intensity", 1 }, "Intensity",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "macro_color", 1 }, "Color",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "macro_space", 1 }, "Space",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "macro_tune", 1 }, "Tune",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));

    // ── Output gain trim (-12 to +12 dB) ────────────────────
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "out_gain", 1 }, "Output Gain",
        juce::NormalisableRange<float> (-12.0f, 12.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("dB")));

    // ── Global bypass & Advanced mode ────────────────────────
    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "global_bypass", 1 }, "Bypass", false));

    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "advanced_mode", 1 }, "Advanced Mode", false));

    // ── Advanced Parameters ──────────────────────────────────
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "gate_thresh", 1 }, "Gate Thresh",
        juce::NormalisableRange<float> (-80.0f, 0.0f, 0.1f), -50.0f));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "comp_thresh", 1 }, "Comp Thresh",
        juce::NormalisableRange<float> (-60.0f, 0.0f, 0.1f), -20.0f));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "comp_ratio", 1 }, "Comp Ratio",
        juce::NormalisableRange<float> (1.0f, 20.0f, 0.1f), 4.0f));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "sat_drive", 1 }, "Sat Drive",
        juce::NormalisableRange<float> (0.0f, 18.0f, 0.1f), 0.0f));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "air_gain", 1 }, "Air Gain",
        juce::NormalisableRange<float> (0.0f, 12.0f, 0.1f), 0.0f));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "chorus_mix", 1 }, "Chorus Mix",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "delay_mix", 1 }, "Delay Mix",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "reverb_wet", 1 }, "Reverb Wet",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "reverb_size", 1 }, "Reverb Size",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f));

    return layout;
}

//==============================================================================
void VocalLabProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sampleRate_ = sampleRate;

    // Smoothers: 50 ms ramp to eliminate zipper noise on knob movement
    smIntensity.reset (sampleRate, 0.05);
    smColor.reset     (sampleRate, 0.05);
    smSpace.reset     (sampleRate, 0.05);
    smOutGain.reset   (sampleRate, 0.05);

    smIntensity.setCurrentAndTargetValue (pIntensity->load());
    smColor.setCurrentAndTargetValue     (pColor->load());
    smSpace.setCurrentAndTargetValue     (pSpace->load());
    smOutGain.setCurrentAndTargetValue   (juce::Decibels::decibelsToGain (pOutGain->load()));

    smGateThresh.reset (sampleRate, 0.05);
    smCompThresh.reset (sampleRate, 0.05);
    smCompRatio.reset  (sampleRate, 0.05);
    smSatDrive.reset   (sampleRate, 0.05);
    smAirGain.reset    (sampleRate, 0.05);
    smChorusMix.reset  (sampleRate, 0.05);
    smDelayMix.reset   (sampleRate, 0.05);
    smReverbWet.reset  (sampleRate, 0.05);
    smReverbSize.reset (sampleRate, 0.05);

    smGateThresh.setCurrentAndTargetValue (pGateThresh->load());
    smCompThresh.setCurrentAndTargetValue (pCompThresh->load());
    smCompRatio.setCurrentAndTargetValue  (pCompRatio->load());
    smSatDrive.setCurrentAndTargetValue   (pSatDrive->load());
    smAirGain.setCurrentAndTargetValue    (pAirGain->load());
    smChorusMix.setCurrentAndTargetValue  (pChorusMix->load());
    smDelayMix.setCurrentAndTargetValue   (pDelayMix->load());
    smReverbWet.setCurrentAndTargetValue  (pReverbWet->load());
    smReverbSize.setCurrentAndTargetValue (pReverbSize->load());

    // Prepare DSP chain
    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels      = (juce::uint32) getTotalNumOutputChannels();

    dspChain.prepare (spec);

    // ── Gate ─────────────────────────────────────────────────
    dspChain.get<kGate>().setThreshold (-50.0f);
    dspChain.get<kGate>().setAttack    (2.0f);
    dspChain.get<kGate>().setRelease   (100.0f);

    // ── High-Pass @ 80 Hz (removes low rumble) ───────────────
    dspChain.get<kHPF>().coefficients =
        juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, 80.0f, 0.707f);

    // ── Compressor defaults ───────────────────────────────────
    dspChain.get<kComp>().setAttack   (5.0f);
    dspChain.get<kComp>().setRelease  (80.0f);
    dspChain.get<kComp>().setThreshold(-20.0f);
    dspChain.get<kComp>().setRatio    (4.0f);

    // ── Air EQ @ 12 kHz (presence shelf) ─────────────────────
    dspChain.get<kAirEQ>().coefficients =
        juce::dsp::IIR::Coefficients<float>::makeHighShelf (sampleRate, 12000.0f, 0.707f, 1.0f);

    // ── Chorus / vocal thickener ──────────────────────────────
    dspChain.get<kChorus>().setRate        (1.2f);
    dspChain.get<kChorus>().setDepth       (0.08f);
    dspChain.get<kChorus>().setCentreDelay (12.0f);
    dspChain.get<kChorus>().setFeedback    (0.0f);
    dspChain.get<kChorus>().setMix         (0.0f);

    // ── Delay ─────────────────────────────────────────────────
    delayLine.prepare (spec);
    delayLine.setMaximumDelayInSamples ((int)(sampleRate * 2.0));

    // ── Reverb ────────────────────────────────────────────────
    juce::Reverb::Parameters rp;
    rp.roomSize  = 0.5f;
    rp.damping   = 0.5f;
    rp.wetLevel  = 0.0f;
    rp.dryLevel  = 1.0f;
    rp.width     = 0.8f;
    rp.freezeMode = 0.0f;
    reverb.setParameters (rp);
    reverb.setSampleRate (sampleRate);
    reverb.reset();

    compGainReductionDb.store (0.0f);
}

void VocalLabProcessor::releaseResources()
{
    reverb.reset();
}

//==============================================================================
void VocalLabProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                      juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;

    const int numIn  = getTotalNumInputChannels();
    const int numOut = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();

    // Clear extra output channels
    for (int ch = numIn; ch < numOut; ++ch)
        buffer.clear (ch, 0, numSamples);

    // ── Bypass ────────────────────────────────────────────────
    if (pBypass->load() > 0.5f)
    {
        compGainReductionDb.store (0.0f);
        // Still meter the signal
        float inRms = buffer.getRMSLevel (0, 0, numSamples);
        inputLevelDb.store  (juce::Decibels::gainToDecibels (inRms, -60.0f));
        outputLevelDb.store (juce::Decibels::gainToDecibels (inRms, -60.0f));
        return;
    }

    // ── Measure input level ───────────────────────────────────
    {
        float rms = 0.0f;
        for (int ch = 0; ch < juce::jmin (numIn, 2); ++ch)
            rms += buffer.getRMSLevel (ch, 0, numSamples);
        rms /= (float) juce::jmin (numIn, 2);
        inputLevelDb.store (juce::Decibels::gainToDecibels (rms, -60.0f));
    }

    // ── Smooth macros ─────────────────────────────────────────
    smIntensity.setTargetValue (pIntensity->load());
    smColor.setTargetValue     (pColor->load());
    smSpace.setTargetValue     (pSpace->load());
    smOutGain.setTargetValue   (juce::Decibels::decibelsToGain (pOutGain->load()));

    smGateThresh.setTargetValue (pGateThresh->load());
    smCompThresh.setTargetValue (pCompThresh->load());
    smCompRatio.setTargetValue  (pCompRatio->load());
    smSatDrive.setTargetValue   (pSatDrive->load());
    smAirGain.setTargetValue    (pAirGain->load());
    smChorusMix.setTargetValue  (pChorusMix->load());
    smDelayMix.setTargetValue   (pDelayMix->load());
    smReverbWet.setTargetValue  (pReverbWet->load());
    smReverbSize.setTargetValue (pReverbSize->load());

    smIntensity.skip (numSamples);
    smColor.skip     (numSamples);
    smSpace.skip     (numSamples);
    smOutGain.skip   (numSamples);

    smGateThresh.skip (numSamples);
    smCompThresh.skip (numSamples);
    smCompRatio.skip  (numSamples);
    smSatDrive.skip   (numSamples);
    smAirGain.skip    (numSamples);
    smChorusMix.skip  (numSamples);
    smDelayMix.skip   (numSamples);
    smReverbWet.skip  (numSamples);
    smReverbSize.skip (numSamples);

    const float intensity = smIntensity.getCurrentValue();
    const float color     = smColor.getCurrentValue();
    const float space     = smSpace.getCurrentValue();
    const float outGain   = smOutGain.getCurrentValue();

    // ── Map macros/advanced → DSP parameters ──────────────────

    float gateThresh, compThresh, compRatio, driveDb, airGainDb, chorusMix, delayMix, reverbWet, roomSize;

    if (pAdvanced->load() > 0.5f)
    {
        // Use individual parameters
        gateThresh = smGateThresh.getCurrentValue();
        compThresh = smCompThresh.getCurrentValue();
        compRatio  = smCompRatio.getCurrentValue();
        driveDb    = smSatDrive.getCurrentValue();
        airGainDb  = smAirGain.getCurrentValue();
        chorusMix  = smChorusMix.getCurrentValue();
        delayMix   = smDelayMix.getCurrentValue();
        reverbWet  = smReverbWet.getCurrentValue();
        roomSize   = smReverbSize.getCurrentValue();
    }
    else
    {
        // Intensity → Gate + Compressor
        gateThresh = juce::jmap (intensity, 0.0f, 1.0f, -30.0f, -60.0f);
        compThresh = juce::jmap (intensity, 0.0f, 1.0f, -10.0f, -40.0f);
        compRatio  = juce::jmap (intensity, 0.0f, 1.0f,  2.0f,  10.0f);

        // Color → Saturation drive + Air EQ + Chorus
        driveDb    = juce::jmap (color, 0.0f, 1.0f, 0.0f, 18.0f);
        airGainDb  = juce::jmap (color, 0.0f, 1.0f, 0.0f, 8.0f);
        chorusMix  = juce::jmap (color, 0.0f, 1.0f, 0.0f, 0.35f);

        // Space → Delay mix + Reverb room
        delayMix   = juce::jmap (space, 0.0f, 1.0f, 0.0f, 0.25f);
        reverbWet  = juce::jmap (space, 0.0f, 1.0f, 0.0f, 0.55f);
        roomSize   = juce::jmap (space, 0.0f, 1.0f, 0.3f, 0.9f);
    }

    dspChain.get<kGate>().setThreshold (gateThresh);
    dspChain.get<kComp>().setThreshold (compThresh);
    dspChain.get<kComp>().setRatio     (compRatio);

    const float preGain  = juce::Decibels::decibelsToGain (driveDb);
    const float postGain = 1.0f / juce::jmax (preGain, 1e-6f);
    const float airGain  = juce::Decibels::decibelsToGain (airGainDb);

    dspChain.get<kAirEQ>().coefficients =
        juce::dsp::IIR::Coefficients<float>::makeHighShelf (sampleRate_, 12000.0f, 0.707f, airGain);
    dspChain.get<kChorus>().setMix (chorusMix);

    const float delaySamples = (250.0f / 1000.0f) * (float)sampleRate_;

    juce::Reverb::Parameters rp = reverb.getParameters();
    rp.wetLevel  = reverbWet;
    rp.dryLevel  = 1.0f;
    rp.roomSize  = roomSize;
    reverb.setParameters (rp);

    // ── Process DSP chain ─────────────────────────────────────
    // 1. Pre-gain (drives WaveShaper)
    buffer.applyGain (preGain);

    // 2. Main serial chain
    juce::dsp::AudioBlock<float>            block (buffer);
    juce::dsp::ProcessContextReplacing<float> ctx (block);
    dspChain.process (ctx);

    // 3. Post-gain (compensate saturation loudness increase)
    buffer.applyGain (postGain);

    // ── Read compressor GR for metering ───────────────────────
    // juce::dsp::Compressor doesn't expose GR directly, approximate from buffer level
    // (production: hook into custom compressor for real GR)
    compGainReductionDb.store (0.0f); // placeholder – update when using custom comp

    // ── Delay line (per-sample to avoid allocations) ──────────
    for (int ch = 0; ch < juce::jmin (numIn, 2); ++ch)
    {
        auto* data = buffer.getWritePointer (ch);
        for (int i = 0; i < numSamples; ++i)
        {
            const float dry = data[i];
            delayLine.pushSample (ch, dry);
            const float delayed = delayLine.popSample (ch, delaySamples);
            data[i] = dry + delayed * delayMix;
        }
    }

    // ── Reverb ────────────────────────────────────────────────
    if (numIn == 1)
        reverb.processMono (buffer.getWritePointer (0), numSamples);
    else if (numIn >= 2)
        reverb.processStereo (buffer.getWritePointer (0), buffer.getWritePointer (1), numSamples);

    // ── Output gain trim ──────────────────────────────────────
    buffer.applyGain (outGain);

    // ── Measure output level ──────────────────────────────────
    {
        float rms = 0.0f;
        for (int ch = 0; ch < juce::jmin (numIn, 2); ++ch)
            rms += buffer.getRMSLevel (ch, 0, numSamples);
        rms /= (float) juce::jmin (numIn, 2);
        outputLevelDb.store (juce::Decibels::gainToDecibels (rms, -60.0f));
    }
}

//==============================================================================
void VocalLabProcessor::parameterChanged (const juce::String& /*paramID*/, float /*newValue*/) {}

//==============================================================================
void VocalLabProcessor::loadArtistPreset (int index)
{
    if (index < 0 || index >= kNumArtistPresets) return;
    const auto& preset = kArtistPresets[index];

    auto setNorm = [&] (const char* id, float v)
    {
        if (auto* p = apvts.getParameter (id))
            p->setValueNotifyingHost (p->convertTo0to1 (v));
    };

    setNorm ("macro_intensity", preset.intensity);
    setNorm ("macro_color",     preset.color);
    setNorm ("macro_space",     preset.space);
    setNorm ("macro_tune",      preset.tune);
}

//==============================================================================
juce::AudioProcessorEditor* VocalLabProcessor::createEditor()
{
    return new VocalLabEditor (*this);
}

void VocalLabProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void VocalLabProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VocalLabProcessor();
}
