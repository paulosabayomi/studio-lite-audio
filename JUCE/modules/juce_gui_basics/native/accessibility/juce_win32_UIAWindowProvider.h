/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
class UIAWindowProvider : public UIAProviderBase,
                          public ComBaseClassHelper<IWindowProvider>
{
public:
    explicit UIAWindowProvider (AccessibilityNativeHandle* nativeHandle)
        : UIAProviderBase (nativeHandle)
    {
    }

    //==============================================================================
    JUCE_COMRESULT SetVisualState (WindowVisualState state) override
    {
        if (! isElementValid())
            return UIA_E_ELEMENTNOTAVAILABLE;

        if (auto* peer = getPeer())
        {
            switch (state)
            {
                case WindowVisualState_Maximized:
                    peer->setFullScreen (true);
                    break;

                case WindowVisualState_Minimized:
                    peer->setMinimised (true);
                    break;

                case WindowVisualState_Normal:
                    peer->setFullScreen (false);
                    peer->setMinimised (false);
                    break;

                default:
                    break;
            }

            return S_OK;
        }

        return UIA_E_NOTSUPPORTED;
    }

    JUCE_COMRESULT Close() override
    {
        if (! isElementValid())
            return UIA_E_ELEMENTNOTAVAILABLE;

        if (auto* peer = getPeer())
        {
            peer->handleUserClosingWindow();
            return S_OK;
        }

        return UIA_E_NOTSUPPORTED;
    }

    JUCE_COMRESULT WaitForInputIdle (int, BOOL* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, []
        {
            return UIA_E_NOTSUPPORTED;
        });
    }

    JUCE_COMRESULT get_CanMaximize (BOOL* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]() -> HRESULT
        {
            if (auto* peer = getPeer())
            {
                *pRetVal = (peer->getStyleFlags() & ComponentPeer::windowHasMaximiseButton) != 0;
                return S_OK;
            }

            return UIA_E_NOTSUPPORTED;
        });
    }

    JUCE_COMRESULT get_CanMinimize (BOOL* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]() -> HRESULT
        {
            if (auto* peer = getPeer())
            {
                *pRetVal = (peer->getStyleFlags() & ComponentPeer::windowHasMinimiseButton) != 0;
                return S_OK;
            }

            return UIA_E_NOTSUPPORTED;
        });
    }

    JUCE_COMRESULT get_IsModal (BOOL* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]() -> HRESULT
        {
            if (auto* peer = getPeer())
            {
                *pRetVal = peer->getComponent().isCurrentlyModal();
                return S_OK;
            }

            return UIA_E_NOTSUPPORTED;
        });
    }

    JUCE_COMRESULT get_WindowVisualState (WindowVisualState* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]() -> HRESULT
        {
            if (auto* peer = getPeer())
            {
                if (peer->isFullScreen())
                    *pRetVal = WindowVisualState_Maximized;
                else if (peer->isMinimised())
                    *pRetVal = WindowVisualState_Minimized;
                else
                    *pRetVal = WindowVisualState_Normal;

                return S_OK;
            }

            return UIA_E_NOTSUPPORTED;
        });
    }

    JUCE_COMRESULT get_WindowInteractionState (WindowInteractionState* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]() -> HRESULT
        {
            if (auto* peer = getPeer())
            {
                *pRetVal = peer->getComponent().isCurrentlyBlockedByAnotherModalComponent()
                    ? WindowInteractionState::WindowInteractionState_BlockedByModalWindow
                    : WindowInteractionState::WindowInteractionState_Running;

                return S_OK;
            }

            return UIA_E_NOTSUPPORTED;
        });
    }

    JUCE_COMRESULT get_IsTopmost (BOOL* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]() -> HRESULT
        {
            if (auto* peer = getPeer())
            {
                *pRetVal = peer->isFocused();
                return S_OK;
            }

            return UIA_E_NOTSUPPORTED;
        });
    }

private:
    ComponentPeer* getPeer() const
    {
        return getHandler().getComponent().getPeer();
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIAWindowProvider)
};

} // namespace juce
