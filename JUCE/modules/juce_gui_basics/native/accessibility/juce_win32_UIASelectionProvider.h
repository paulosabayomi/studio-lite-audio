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

JUCE_COMCLASS (ISelectionProvider2, "14f68475-ee1c-44f6-a869-d239381f0fe7")  : public ISelectionProvider
{
    JUCE_COMCALL get_FirstSelectedItem   (IRawElementProviderSimple** retVal) = 0;
    JUCE_COMCALL get_LastSelectedItem    (IRawElementProviderSimple** retVal) = 0;
    JUCE_COMCALL get_CurrentSelectedItem (IRawElementProviderSimple** retVal) = 0;
    JUCE_COMCALL get_ItemCount (int* retVal) = 0;
};

//==============================================================================
class UIASelectionItemProvider  : public UIAProviderBase,
                                  public ComBaseClassHelper<ISelectionItemProvider>
{
public:
    explicit UIASelectionItemProvider (AccessibilityNativeHandle* nativeHandle)
        : UIAProviderBase (nativeHandle),
          isRadioButton (getHandler().getRole() == AccessibilityRole::radioButton)
    {
    }

    //==============================================================================
    JUCE_COMRESULT AddToSelection() override
    {
        if (! isElementValid())
            return UIA_E_ELEMENTNOTAVAILABLE;

        const auto& handler = getHandler();

        if (isRadioButton)
        {
            handler.getActions().invoke (AccessibilityActionType::press);
            sendAccessibilityAutomationEvent (handler, UIA_SelectionItem_ElementSelectedEventId);

            return S_OK;
        }

        handler.getActions().invoke (AccessibilityActionType::toggle);
        handler.getActions().invoke (AccessibilityActionType::press);

        return S_OK;
    }

    JUCE_COMRESULT get_IsSelected (BOOL* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            const auto state = getHandler().getCurrentState();
            *pRetVal = isRadioButton ? state.isChecked() : state.isSelected();
            return S_OK;
        });
    }

    JUCE_COMRESULT get_SelectionContainer (IRawElementProviderSimple** pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            if (! isRadioButton)
                if (auto* parent = getHandler().getParent())
                    parent->getNativeImplementation()->QueryInterface (IID_PPV_ARGS (pRetVal));

            return S_OK;
        });
    }

    JUCE_COMRESULT RemoveFromSelection() override
    {
        if (! isElementValid())
            return UIA_E_ELEMENTNOTAVAILABLE;

        if (! isRadioButton)
        {
            const auto& handler = getHandler();

            if (handler.getCurrentState().isSelected())
                getHandler().getActions().invoke (AccessibilityActionType::toggle);
        }

        return S_OK;
    }

    JUCE_COMRESULT Select() override
    {
        if (! isElementValid())
            return UIA_E_ELEMENTNOTAVAILABLE;

        AddToSelection();

        if (isElementValid() && ! isRadioButton)
        {
            const auto& handler = getHandler();

            if (auto* parent = handler.getParent())
                for (auto* child : parent->getChildren())
                    if (child != &handler && child->getCurrentState().isSelected())
                        child->getActions().invoke (AccessibilityActionType::toggle);
        }

        return S_OK;
    }

private:
    const bool isRadioButton;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIASelectionItemProvider)
};

//==============================================================================
class UIASelectionProvider  : public UIAProviderBase,
                              public ComBaseClassHelper<ISelectionProvider2>
{
public:
    explicit UIASelectionProvider (AccessibilityNativeHandle* nativeHandle)
        : UIAProviderBase (nativeHandle)
    {
    }

    //==============================================================================
    JUCE_COMRESULT QueryInterface (REFIID iid, void** result) override
    {
        if (iid == _uuidof (IUnknown) || iid == _uuidof (ISelectionProvider))
            return castToType<ISelectionProvider> (result);

        if (iid == _uuidof (ISelectionProvider2))
            return castToType<ISelectionProvider2> (result);

        *result = nullptr;
        return E_NOINTERFACE;
    }

    //==============================================================================
    JUCE_COMRESULT get_CanSelectMultiple (BOOL* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            *pRetVal = isMultiSelectable();
            return S_OK;
        });
    }

    JUCE_COMRESULT get_IsSelectionRequired (BOOL* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            *pRetVal = getSelectedChildren().size() > 0 && ! isMultiSelectable();
            return S_OK;
        });
    }

    JUCE_COMRESULT GetSelection (SAFEARRAY** pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            return addHandlersToArray (getSelectedChildren(), pRetVal);
        });
    }

    //==============================================================================
    JUCE_COMRESULT get_FirstSelectedItem (IRawElementProviderSimple** pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            auto selectedChildren = getSelectedChildren();

            if (! selectedChildren.empty())
                selectedChildren.front()->getNativeImplementation()->QueryInterface (IID_PPV_ARGS (pRetVal));

            return S_OK;
        });
    }

    JUCE_COMRESULT get_LastSelectedItem (IRawElementProviderSimple** pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            auto selectedChildren = getSelectedChildren();

            if (! selectedChildren.empty())
                selectedChildren.back()->getNativeImplementation()->QueryInterface (IID_PPV_ARGS (pRetVal));

            return S_OK;
        });
    }

    JUCE_COMRESULT get_CurrentSelectedItem (IRawElementProviderSimple** pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            get_FirstSelectedItem (pRetVal);
            return S_OK;
        });
    }

    JUCE_COMRESULT get_ItemCount (int* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            *pRetVal = (int) getSelectedChildren().size();
            return S_OK;
        });
    }

private:
    bool isMultiSelectable() const noexcept
    {
         return getHandler().getCurrentState().isMultiSelectable();
    }

    std::vector<const AccessibilityHandler*> getSelectedChildren() const
    {
        std::vector<const AccessibilityHandler*> selectedHandlers;

        for (auto* child : getHandler().getComponent().getChildren())
            if (auto* handler = child->getAccessibilityHandler())
                if (handler->getCurrentState().isSelected())
                    selectedHandlers.push_back (handler);

        return selectedHandlers;
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIASelectionProvider)
};

} // namespace juce
