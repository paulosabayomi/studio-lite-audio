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
class UIAGridItemProvider  : public UIAProviderBase,
                             public ComBaseClassHelper<IGridItemProvider>
{
public:
    explicit UIAGridItemProvider (AccessibilityNativeHandle* nativeHandle)
        : UIAProviderBase (nativeHandle)
    {
    }

    //==============================================================================
    JUCE_COMRESULT get_Row (int* pRetVal) override
    {
        return withCellInterface (pRetVal, [&] (const AccessibilityCellInterface& cellInterface)
        {
            *pRetVal = cellInterface.getRowIndex();
        });
    }

    JUCE_COMRESULT get_Column (int* pRetVal) override
    {
        return withCellInterface (pRetVal, [&] (const AccessibilityCellInterface& cellInterface)
        {
            *pRetVal = cellInterface.getColumnIndex();
        });
    }

    JUCE_COMRESULT get_RowSpan (int* pRetVal) override
    {
        return withCellInterface (pRetVal, [&] (const AccessibilityCellInterface& cellInterface)
        {
            *pRetVal = cellInterface.getRowSpan();
        });
    }

    JUCE_COMRESULT get_ColumnSpan (int* pRetVal) override
    {
        return withCellInterface (pRetVal, [&] (const AccessibilityCellInterface& cellInterface)
        {
            *pRetVal = cellInterface.getColumnSpan();
        });
    }

    JUCE_COMRESULT get_ContainingGrid (IRawElementProviderSimple** pRetVal) override
    {
        return withCellInterface (pRetVal, [&] (const AccessibilityCellInterface& cellInterface)
        {
            if (auto* handler = cellInterface.getTableHandler())
                handler->getNativeImplementation()->QueryInterface (IID_PPV_ARGS (pRetVal));
        });
    }

private:
    template <typename Value, typename Callback>
    JUCE_COMRESULT withCellInterface (Value* pRetVal, Callback&& callback) const
    {
        return withCheckedComArgs (pRetVal, *this, [&]() -> HRESULT
        {
            if (auto* cellInterface = getHandler().getCellInterface())
            {
                callback (*cellInterface);
                return S_OK;
            }

            return UIA_E_NOTSUPPORTED;
        });
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIAGridItemProvider)
};

} // namespace juce
