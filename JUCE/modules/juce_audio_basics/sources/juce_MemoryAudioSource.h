/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    An AudioSource which takes some float audio data as an input.

    @tags{Audio}
*/
class JUCE_API MemoryAudioSource   : public PositionableAudioSource
{
public:
    //==============================================================================
    /**  Creates a MemoryAudioSource by providing an audio buffer.

         If copyMemory is true then the buffer will be copied into an internal
         buffer which will be owned by the MemoryAudioSource. If copyMemory is
         false, then you must ensure that the lifetime of the audio buffer is
         at least as long as the MemoryAudioSource.
    */
    MemoryAudioSource (AudioBuffer<float>& audioBuffer, bool copyMemory, bool shouldLoop = false);

    //==============================================================================
    /** Implementation of the AudioSource method. */
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;

    /** Implementation of the AudioSource method. */
    void releaseResources() override;

    /** Implementation of the AudioSource method. */
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;

    //==============================================================================
    /** Implementation of the PositionableAudioSource method. */
    void setNextReadPosition (int64 newPosition) override;

    /** Implementation of the PositionableAudioSource method. */
    int64 getNextReadPosition() const override;

    /** Implementation of the PositionableAudioSource method. */
    int64 getTotalLength() const override;

    //==============================================================================    
    /** Implementation of the PositionableAudioSource method. */
    bool isLooping() const override;

    /** Implementation of the PositionableAudioSource method. */
    void setLooping (bool shouldLoop) override;

    /** Sets the start position of the looping in samples. */
    void setLoopRange (int64 loopStart, int64 loopLength) override;
    
    /** Returns the position where the loop playback starts.  */
    void getLoopRange(int64 & loopStart, int64 & loopLength) const override;
    
private:
    //==============================================================================
    AudioBuffer<float> buffer;
    int position = 0;
    bool isCurrentlyLooping;
    int64 loopStartPos = 0;
    int64 loopLen = 0;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MemoryAudioSource)
};

} // namespace juce
