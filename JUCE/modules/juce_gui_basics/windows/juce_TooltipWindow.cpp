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

TooltipWindow::TooltipWindow (Component* parentComp, int delayMs)
    : Component ("tooltip"),
      millisecondsBeforeTipAppears (delayMs)
{
    setAlwaysOnTop (true);
    setOpaque (true);

    if (parentComp != nullptr)
        parentComp->addChildComponent (this);

    if (Desktop::getInstance().getMainMouseSource().canHover())
        startTimer (123);
}

TooltipWindow::~TooltipWindow()
{
    hideTip();
}

void TooltipWindow::setMillisecondsBeforeTipAppears (const int newTimeMs) noexcept
{
    millisecondsBeforeTipAppears = newTimeMs;
}

void TooltipWindow::paint (Graphics& g)
{
    getLookAndFeel().drawTooltip (g, tipShowing, getWidth(), getHeight());
}

void TooltipWindow::mouseEnter (const MouseEvent&)
{
    hideTip();
}

void TooltipWindow::updatePosition (const String& tip, Point<int> pos, Rectangle<int> parentArea)
{
    setBounds (getLookAndFeel().getTooltipBounds (tip, pos, parentArea));
    setVisible (true);
}

#if JUCE_DEBUG
static Array<TooltipWindow*> activeTooltipWindows;
#endif

void TooltipWindow::displayTip (Point<int> screenPos, const String& tip)
{
    jassert (tip.isNotEmpty());

    if (! reentrant)
    {
        ScopedValueSetter<bool> setter (reentrant, true, false);

        if (tipShowing != tip)
        {
            tipShowing = tip;
            repaint();
        }

        if (auto* parent = getParentComponent())
        {
            updatePosition (tip, parent->getLocalPoint (nullptr, screenPos),
                            parent->getLocalBounds());
        }
        else
        {
            const auto physicalPos = ScalingHelpers::scaledScreenPosToUnscaled (screenPos);
            const auto scaledPos = ScalingHelpers::unscaledScreenPosToScaled (*this, physicalPos);
            updatePosition (tip, scaledPos, Desktop::getInstance().getDisplays().getDisplayForPoint (screenPos)->userArea);

            addToDesktop (ComponentPeer::windowHasDropShadow
                          | ComponentPeer::windowIsTemporary
                          | ComponentPeer::windowIgnoresKeyPresses
                          | ComponentPeer::windowIgnoresMouseClicks);
        }

       #if JUCE_DEBUG
        activeTooltipWindows.addIfNotAlreadyThere (this);

        auto* parent = getParentComponent();

        for (auto* w : activeTooltipWindows)
        {
            if (w != nullptr && w != this && w->tipShowing == tipShowing && w->getParentComponent() == parent)
            {
                // Looks like you have more than one TooltipWindow showing the same tip..
                // Be careful not to create more than one instance of this class with the
                // same parent component!
                jassertfalse;
            }
        }
       #endif

        toFront (false);

        if (auto* handler = getAccessibilityHandler())
        {
            setDescription (tip);
            handler->grabFocus();
        }
    }
}

String TooltipWindow::getTipFor (Component& c)
{
    if (Process::isForegroundProcess()
         && ! ModifierKeys::currentModifiers.isAnyMouseButtonDown())
    {
        if (auto* ttc = dynamic_cast<TooltipClient*> (&c))
            if (! c.isCurrentlyBlockedByAnotherModalComponent())
                return ttc->getTooltip();
    }

    return {};
}

void TooltipWindow::hideTip()
{
    if (! reentrant)
    {
        if (auto* handler = getAccessibilityHandler())
            handler->giveAwayFocus();

        tipShowing.clear();
        removeFromDesktop();
        setVisible (false);

       #if JUCE_DEBUG
        activeTooltipWindows.removeAllInstancesOf (this);
       #endif
    }
}

float TooltipWindow::getDesktopScaleFactor() const
{
    if (lastComponentUnderMouse != nullptr)
        return Component::getApproximateScaleFactorForComponent (lastComponentUnderMouse);

    return Component::getDesktopScaleFactor();
}

void TooltipWindow::timerCallback()
{
    auto& desktop = Desktop::getInstance();
    auto mouseSource = desktop.getMainMouseSource();
    auto now = Time::getApproximateMillisecondCounter();

    auto* newComp = mouseSource.isTouch() ? nullptr : mouseSource.getComponentUnderMouse();

    if (newComp == nullptr || getParentComponent() == nullptr || newComp->getPeer() == getPeer())
    {
        auto newTip = newComp != nullptr ? getTipFor (*newComp) : String();
        bool tipChanged = (newTip != lastTipUnderMouse || newComp != lastComponentUnderMouse);
        lastComponentUnderMouse = newComp;
        lastTipUnderMouse = newTip;

        auto clickCount = desktop.getMouseButtonClickCounter();
        auto wheelCount = desktop.getMouseWheelMoveCounter();
        bool mouseWasClicked = (clickCount > mouseClicks || wheelCount > mouseWheelMoves);
        mouseClicks = clickCount;
        mouseWheelMoves = wheelCount;

        auto mousePos = mouseSource.getScreenPosition();
        bool mouseMovedQuickly = mousePos.getDistanceFrom (lastMousePos) > 12;
        lastMousePos = mousePos;

        if (tipChanged || mouseWasClicked || mouseMovedQuickly)
            lastCompChangeTime = now;

        if (isVisible() || now < lastHideTime + 500)
        {
            // if a tip is currently visible (or has just disappeared), update to a new one
            // immediately if needed..
            if (newComp == nullptr || mouseWasClicked || newTip.isEmpty())
            {
                if (isVisible())
                {
                    lastHideTime = now;
                    hideTip();
                }
            }
            else if (tipChanged)
            {
                displayTip (mousePos.roundToInt(), newTip);
            }
        }
        else
        {
            // if there isn't currently a tip, but one is needed, only let it
            // appear after a timeout..
            if (newTip.isNotEmpty()
                 && newTip != tipShowing
                 && now > lastCompChangeTime + (uint32) millisecondsBeforeTipAppears)
            {
                displayTip (mousePos.roundToInt(), newTip);
            }
        }
    }
}

//==============================================================================
std::unique_ptr<AccessibilityHandler> TooltipWindow::createAccessibilityHandler()
{
    return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::tooltip);
}

} // namespace juce
