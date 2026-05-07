#include "DeepSeekBalanceItem.h"
#include "DataManager.h"

CDeepSeekBalanceItem::CDeepSeekBalanceItem()
{
}

const wchar_t* CDeepSeekBalanceItem::GetItemName() const
{
    return L"DeepSeek Balance";
}

const wchar_t* CDeepSeekBalanceItem::GetItemId() const
{
    return L"DeepSeekBalance_v1";
}

const wchar_t* CDeepSeekBalanceItem::GetItemLableText() const
{
    m_label_text = L"DS";
    return m_label_text.c_str();
}

const wchar_t* CDeepSeekBalanceItem::GetItemValueText() const
{
    const auto& data = CDataManager::Instance().GetBalanceData();
    if (!data.has_data)
    {
        m_value_text = L"--";
        return m_value_text.c_str();
    }

    // Display format: "8.87 CNY"
    m_value_text = data.balance_info.total_balance;
    if (!data.balance_info.currency.empty())
    {
        m_value_text += L" ";
        m_value_text += data.balance_info.currency;
    }
    return m_value_text.c_str();
}

const wchar_t* CDeepSeekBalanceItem::GetItemValueSampleText() const
{
    return L"888.88 CNY";
}
