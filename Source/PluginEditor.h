#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
//  Custom LookAndFeel — dark, premium aesthetic
//==============================================================================
class VocalLabLAF : public juce::LookAndFeel_V4
{
public:
    // Colour palette
    static constexpr juce::uint32 ColBg         = 0xff141417;
    static constexpr juce::uint32 ColPanel       = 0xff1e1e24;
    static constexpr juce::uint32 ColCard        = 0xff26262e;
    static constexpr juce::uint32 ColAccent      = 0xff7c5bfc; // purple
    static constexpr juce::uint32 ColAccentHot   = 0xff9d7dff;
    static constexpr juce::uint32 ColText        = 0xfff0eeff;
    static constexpr juce::uint32 ColTextDim     = 0xff888899;
    static constexpr juce::uint32 ColKnobRim     = 0xff3a3a48;
    static constexpr juce::uint32 ColGreen       = 0xff3ddc84;

    VocalLabLAF()
    {
        setColour (juce::ComboBox::backgroundColourId,     juce::Colour (ColCard));
        setColour (juce::ComboBox::outlineColourId,        juce::Colour (ColKnobRim));
        setColour (juce::ComboBox::textColourId,           juce::Colour (ColText));
        setColour (juce::ComboBox::arrowColourId,          juce::Colour (ColAccent));
        setColour (juce::PopupMenu::backgroundColourId,    juce::Colour (ColCard));
        setColour (juce::PopupMenu::textColourId,          juce::Colour (ColText));
        setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (ColAccent));
        setColour (juce::TextButton::buttonColourId,       juce::Colour (ColCard));
        setColour (juce::TextButton::buttonOnColourId,     juce::Colour (ColAccent));
        setColour (juce::TextButton::textColourOffId,      juce::Colour (ColTextDim));
        setColour (juce::TextButton::textColourOnId,       juce::Colour (ColText));
        setColour (juce::Slider::thumbColourId,            juce::Colour (ColAccent));
        setColour (juce::Slider::trackColourId,            juce::Colour (ColAccent));
    }

    // ── Rotary knob ──────────────────────────────────────────
    void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                           float sliderPos, float startAngle, float endAngle,
                           juce::Slider& /*slider*/) override
    {
        const float cx = x + w * 0.5f, cy = y + h * 0.5f;
        const float radius = juce::jmin (w, h) * 0.5f - 4.0f;

        // Shadow
        juce::ColourGradient shadow (juce::Colour (0x55000000), cx, cy + radius * 0.5f,
                                     juce::Colours::transparentBlack, cx, cy - radius, true);
        g.setGradientFill (shadow);
        g.fillEllipse (cx - radius - 2, cy - radius + 4, (radius + 2) * 2, (radius + 2) * 2);

        // Body gradient
        juce::ColourGradient bodyGrad (juce::Colour (0xff35353f), cx, cy - radius,
                                       juce::Colour (0xff1c1c22), cx, cy + radius, false);
        g.setGradientFill (bodyGrad);
        g.fillEllipse (cx - radius, cy - radius, radius * 2, radius * 2);

        // Rim
        g.setColour (juce::Colour (ColKnobRim));
        g.drawEllipse (cx - radius, cy - radius, radius * 2, radius * 2, 1.5f);

        const float trackR = radius - 7.0f;

        // Arc track (background)
        juce::Path track;
        track.addArc (cx - trackR, cy - trackR, trackR * 2, trackR * 2,
                      startAngle, endAngle, true);
        g.setColour (juce::Colour (0xff2d2d38));
        g.strokePath (track, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved,
                                                    juce::PathStrokeType::rounded));

        // Arc value (filled portion up to current position)
        const float valueAngle = startAngle + sliderPos * (endAngle - startAngle);
        if (sliderPos > 0.0f)
        {
            juce::Path arc;
            arc.addArc (cx - trackR, cy - trackR, trackR * 2, trackR * 2,
                        startAngle, valueAngle, true);
            g.setColour (juce::Colour (ColAccent));
            g.strokePath (arc, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved,
                                                      juce::PathStrokeType::rounded));
        }

        // Pointer — thin line pointing outward from centre at valueAngle
        // valueAngle is in JUCE rotary convention: 0 = top (12 o'clock), clockwise positive
        // AffineTransform::rotation also: 0 = right, clockwise positive in screen coords
        // => subtract halfPi to align "up" with 12 o'clock start
        {
            const float angle = valueAngle - juce::MathConstants<float>::halfPi;
            const float innerR = radius * 0.25f;
            const float outerR = radius * 0.82f;
            const float x1 = cx + innerR * std::cos (angle);
            const float y1 = cy + innerR * std::sin (angle);
            const float x2 = cx + outerR * std::cos (angle);
            const float y2 = cy + outerR * std::sin (angle);

            g.setColour (juce::Colour (ColAccentHot));
            g.drawLine (x1, y1, x2, y2, 3.0f);

            // dot at tip
            g.fillEllipse (x2 - 3.0f, y2 - 3.0f, 6.0f, 6.0f);
        }

        // Inner highlight
        juce::ColourGradient hl (juce::Colour (0x22ffffff), cx, cy - radius * 0.4f,
                                  juce::Colours::transparentWhite, cx, cy + radius * 0.2f, false);
        g.setGradientFill (hl);
        g.fillEllipse (cx - radius * 0.7f, cy - radius * 0.65f, radius * 1.4f, radius * 0.85f);
    }

    // ── TextButton (Bypass button style) ─────────────────────
    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                const juce::Colour& /*bg*/, bool hover, bool down) override
    {
        const auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);
        const bool on = button.getToggleState();

        juce::Colour fill = on ? juce::Colour (ColAccent) : juce::Colour (ColCard);
        if (hover) fill = fill.brighter (0.1f);
        if (down)  fill = fill.darker   (0.15f);

        g.setColour (fill);
        g.fillRoundedRectangle (bounds, 6.0f);
        g.setColour (juce::Colour (ColKnobRim));
        g.drawRoundedRectangle (bounds, 6.0f, 1.0f);
    }
};

//==============================================================================
//  LevelMeter — simple vertical RMS bar
//==============================================================================
class LevelMeter : public juce::Component, private juce::Timer
{
public:
    enum Channel { Input, Output };
    LevelMeter (VocalLabProcessor& p, Channel ch) : proc (p), channel (ch)
    {
        startTimerHz (30);
    }
    void timerCallback() override { repaint(); }
    void paint (juce::Graphics& g) override
    {
        const auto b = getLocalBounds().toFloat();
        g.setColour (juce::Colour (VocalLabLAF::ColCard));
        g.fillRoundedRectangle (b, 3.0f);

        const float db   = (channel == Input) ? proc.getInputLevelDb() : proc.getOutputLevelDb();
        const float norm = juce::jmap (juce::jlimit (-60.0f, 0.0f, db), -60.0f, 0.0f, 0.0f, 1.0f);

        juce::ColourGradient grad (juce::Colour (VocalLabLAF::ColGreen), b.getX(), b.getBottom(),
                                   juce::Colours::red, b.getX(), b.getY(), false);
        grad.addColour (0.75, juce::Colours::yellow);
        g.setGradientFill (grad);
        const float fillH = b.getHeight() * norm;
        g.fillRoundedRectangle (b.withTop (b.getBottom() - fillH), 3.0f);

        // dB label
        g.setColour (juce::Colour (VocalLabLAF::ColTextDim));
        g.setFont (juce::FontOptions (8.5f));
        g.drawText (juce::String ((int)db) + "dB", b.withHeight (14).withY (b.getBottom() - 14),
                    juce::Justification::centred);
    }
private:
    VocalLabProcessor& proc;
    Channel channel;
};


class GainReductionMeter : public juce::Component, private juce::Timer
{
public:
    GainReductionMeter (VocalLabProcessor& p) : proc (p)
    {
        startTimerHz (30);
    }
    void timerCallback() override { repaint(); }
    void paint (juce::Graphics& g) override
    {
        const auto b = getLocalBounds().toFloat();
        g.setColour (juce::Colour (VocalLabLAF::ColCard));
        g.fillRoundedRectangle (b, 3.0f);

        const float gr = juce::jlimit (0.0f, 30.0f, proc.getCompGainReduction());
        const float fill = gr / 30.0f;

        juce::ColourGradient grad (juce::Colour (VocalLabLAF::ColGreen), b.getX(), b.getY(),
                                   juce::Colours::red, b.getRight(), b.getY(), false);
        grad.addColour (0.6, juce::Colours::yellow);
        g.setGradientFill (grad);
        g.fillRoundedRectangle (b.withWidth (b.getWidth() * fill), 3.0f);
    }

private:
    VocalLabProcessor& proc;
};

//==============================================================================
//  MacroKnob — labelled rotary slot
//==============================================================================
struct MacroKnob : public juce::Component
{
    juce::Slider  slider;
    juce::Label   label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attach;

    MacroKnob (const juce::String& name,
               juce::AudioProcessorValueTreeState& apvts,
               const juce::String& paramID)
    {
        slider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        slider.setPopupDisplayEnabled (true, false, nullptr);
        addAndMakeVisible (slider);

        label.setText (name, juce::dontSendNotification);
        label.setFont (juce::FontOptions (11.0f).withStyle ("Bold"));
        label.setColour (juce::Label::textColourId, juce::Colour (VocalLabLAF::ColTextDim));
        label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (label);

        attach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            apvts, paramID, slider);
    }

    void resized() override
    {
        const int labelH = 18;
        label .setBounds (0, getHeight() - labelH, getWidth(), labelH);
        slider.setBounds (0, 0, getWidth(), getHeight() - labelH - 4);
    }
};

//==============================================================================
//  Editor
//==============================================================================
class VocalLabEditor : public juce::AudioProcessorEditor,
                       private juce::Timer
{
public:
    VocalLabEditor (VocalLabProcessor&);
    ~VocalLabEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override { repaint(); }  // for animated elements
    void buildPresetBox();
    void updateAdvancedModeState();

    VocalLabProcessor& proc;
    VocalLabLAF        laf;

    // Header
    juce::ComboBox    presetBox;
    juce::TextButton  bypassBtn { "BYPASS" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttach;

    juce::TextButton  advancedBtn { "ADVANCED" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> advancedAttach;

    // Macro knobs + out gain
    MacroKnob knobIntensity;
    MacroKnob knobColor;
    MacroKnob knobSpace;
    MacroKnob knobTune;
    MacroKnob knobOutGain;

    // Level meters
    LevelMeter meterIn;
    LevelMeter meterOut;

    // GR meter
    GainReductionMeter grMeter;

    // Advanced parameters
    MacroKnob knobGateThresh;
    MacroKnob knobCompThresh;
    MacroKnob knobCompRatio;
    MacroKnob knobSatDrive;
    MacroKnob knobAirGain;
    MacroKnob knobChorusMix;
    MacroKnob knobDelayMix;
    MacroKnob knobReverbWet;
    MacroKnob knobReverbSize;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VocalLabEditor)
};
