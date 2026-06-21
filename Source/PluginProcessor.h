#pragma once
#include <JuceHeader.h>

// ============================================================
//  Artist preset data
// ============================================================
struct ArtistPreset
{
    juce::String name;
    float intensity; // 0-1
    float color;     // 0-1
    float space;     // 0-1
    float tune;      // 0-1
};

static const ArtistPreset kArtistPresets[] =
{
    { "Default",        0.5f,  0.5f,  0.5f,  0.0f },
    { "The Weekday",    0.80f, 0.60f, 0.85f, 0.3f },
    { "Billie Eyelash", 0.30f, 0.80f, 0.20f, 0.0f },
    { "Travis S.",      0.90f, 0.90f, 0.60f, 0.5f },
    { "Adella",         0.50f, 0.40f, 0.80f, 0.0f },
    { "Drizzy",         0.70f, 0.70f, 0.50f, 0.4f },
    { "Ariana Venti",   0.55f, 0.55f, 0.65f, 0.2f },
    { "Heruwa (Shaggydog)", 0.60f, 0.75f, 0.40f, 0.1f },
};

static const int kNumArtistPresets = (int)(sizeof(kArtistPresets) / sizeof(kArtistPresets[0]));

// ============================================================
//  Processor
// ============================================================
class VocalLabProcessor  : public juce::AudioProcessor,
                           public juce::AudioProcessorValueTreeState::Listener
{
public:
    VocalLabProcessor();
    ~VocalLabProcessor() override;

    //=========================================================
    // AudioProcessor overrides
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Vocal Lab"; }
    bool acceptsMidi()  const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; } // reverb tail

    int  getNumPrograms() override { return 1; }
    int  getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //=========================================================
    // APVTS
    juce::AudioProcessorValueTreeState apvts;

    // Preset engine
    void loadArtistPreset (int presetIndex);

    // GR meter read (UI reads this)
    float getCompGainReduction() const noexcept { return compGainReductionDb.load(); }
    float getInputLevelDb()  const noexcept { return inputLevelDb.load(); }
    float getOutputLevelDb() const noexcept { return outputLevelDb.load(); }

private:
    //=========================================================
    // APVTS listener
    void parameterChanged (const juce::String& paramID, float newValue) override;

    // Parameter layout
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    //=========================================================
    // Atomic pointers to parameters (fast audio-thread reads)
    std::atomic<float>* pIntensity  = nullptr;
    std::atomic<float>* pColor      = nullptr;
    std::atomic<float>* pSpace      = nullptr;
    std::atomic<float>* pTune       = nullptr;
    std::atomic<float>* pBypass     = nullptr;
    std::atomic<float>* pOutGain    = nullptr;

    // Advanced mode parameters
    std::atomic<float>* pAdvanced   = nullptr;
    std::atomic<float>* pGateThresh = nullptr;
    std::atomic<float>* pCompThresh = nullptr;
    std::atomic<float>* pCompRatio  = nullptr;
    std::atomic<float>* pSatDrive   = nullptr;
    std::atomic<float>* pAirGain    = nullptr;
    std::atomic<float>* pChorusMix  = nullptr;
    std::atomic<float>* pDelayMix   = nullptr;
    std::atomic<float>* pReverbWet  = nullptr;
    std::atomic<float>* pReverbSize = nullptr;

    // Block-level smoothers (ramps over 50 ms to eliminate zipper noise)
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smIntensity;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smColor;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smSpace;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smOutGain;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smGateThresh;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smCompThresh;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smCompRatio;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smSatDrive;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smAirGain;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smChorusMix;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smDelayMix;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smReverbWet;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smReverbSize;

    //=========================================================
    // DSP chain: Gate -> HPF -> Compressor -> WaveShaper -> AirEQ -> Chorus
    enum ChainIdx { kGate = 0, kHPF, kComp, kSaturator, kAirEQ, kChorus };
    juce::dsp::ProcessorChain<
        juce::dsp::NoiseGate<float>,
        juce::dsp::IIR::Filter<float>,
        juce::dsp::Compressor<float>,
        juce::dsp::WaveShaper<float, std::function<float(float)>>,
        juce::dsp::IIR::Filter<float>,
        juce::dsp::Chorus<float>
    > dspChain;

    // Reverb (juce::Reverb – uses processMono/processStereo)
    juce::Reverb reverb;

    // Stereo delay line (2 channels, max 2 s)
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine { 96001 * 2 };

    double sampleRate_ = 44100.0;

    // GR meter (written by audio thread, read by UI)
    std::atomic<float> compGainReductionDb { 0.0f };
    std::atomic<float> inputLevelDb        { -60.0f };
    std::atomic<float> outputLevelDb       { -60.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VocalLabProcessor)
};
