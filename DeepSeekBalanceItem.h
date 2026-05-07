#pragma once
#include <string>
#include "PluginInterface.h"

class CDeepSeekBalanceItem : public IPluginItem
{
public:
    CDeepSeekBalanceItem();

    // IPluginItem
    virtual const wchar_t* GetItemName() const override;
    virtual const wchar_t* GetItemId() const override;
    virtual const wchar_t* GetItemLableText() const override;
    virtual const wchar_t* GetItemValueText() const override;
    virtual const wchar_t* GetItemValueSampleText() const override;

private:
    mutable std::wstring m_label_text;
    mutable std::wstring m_value_text;
};
