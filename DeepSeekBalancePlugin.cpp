#include "DeepSeekBalancePlugin.h"
#include "DataManager.h"
#include "OptionsDlg.h"
#include <windows.h>

CDeepSeekBalancePlugin CDeepSeekBalancePlugin::m_instance;

CDeepSeekBalancePlugin::CDeepSeekBalancePlugin()
{
}

CDeepSeekBalancePlugin& CDeepSeekBalancePlugin::Instance()
{
    return m_instance;
}

IPluginItem* CDeepSeekBalancePlugin::GetItem(int index)
{
    switch (index)
    {
    case 0:
        return &m_balance_item;
    default:
        break;
    }
    return nullptr;
}

void CDeepSeekBalancePlugin::DataRequired()
{
    auto& config = CDataManager::Instance().GetConfig();
    auto& data_mgr = CDataManager::Instance();

    // Check if we need to refresh based on interval
    long long now = GetTickCount64();
    long long last_refresh = data_mgr.GetLastRefreshTime();
    long long elapsed_sec = (now - last_refresh) / 1000;

    if (last_refresh == 0 || elapsed_sec >= config.refresh_interval_sec)
    {
        data_mgr.RefreshBalance();
    }
}

const wchar_t* CDeepSeekBalancePlugin::GetInfo(PluginInfoIndex index)
{
    switch (index)
    {
    case TMI_NAME:
        return L"DeepSeek Balance";
    case TMI_DESCRIPTION:
        return L"显示 DeepSeek API 账户余额";
    case TMI_AUTHOR:
        return L"Claude";
    case TMI_COPYRIGHT:
        return L"";
    case TMI_VERSION:
        return L"1.0";
    case TMI_URL:
        return L"https://github.com/zhongyang219/TrafficMonitor";
    default:
        break;
    }
    return L"";
}

ITMPlugin::OptionReturn CDeepSeekBalancePlugin::ShowOptionsDialog(void* hParent)
{
    COptionsDlg dlg((HWND)hParent);
    if (dlg.DoModal())
    {
        return ITMPlugin::OR_OPTION_CHANGED;
    }
    return ITMPlugin::OR_OPTION_UNCHANGED;
}

const wchar_t* CDeepSeekBalancePlugin::GetTooltipInfo()
{
    const auto& data = CDataManager::Instance().GetBalanceData();
    if (!data.has_data)
    {
        return L"DeepSeek Balance\n暂无数据，请在插件选项中配置 API Token";
    }

    // Build detailed tooltip
    static std::wstring tooltip;
    tooltip = L"DeepSeek 账户余额\n";
    tooltip += L"可用: " + std::wstring(data.is_available ? L"是" : L"否") + L"\n";
    tooltip += L"总额: " + data.balance_info.total_balance + L" " + data.balance_info.currency + L"\n";
    tooltip += L"充值: " + data.balance_info.topped_up_balance + L" " + data.balance_info.currency + L"\n";
    tooltip += L"赠送: " + data.balance_info.granted_balance + L" " + data.balance_info.currency + L"\n";
    if (!data.last_update_time.empty())
    {
        tooltip += L"更新: " + data.last_update_time;
    }
    return tooltip.c_str();
}

void CDeepSeekBalancePlugin::OnExtenedInfo(ExtendedInfoIndex index, const wchar_t* data)
{
    switch (index)
    {
    case EI_CONFIG_DIR:
        CDataManager::Instance().LoadConfig(std::wstring(data));
        // Initial refresh on load
        CDataManager::Instance().RefreshBalance();
        break;
    default:
        break;
    }
}

ITMPlugin* TMPluginGetInstance()
{
    return &CDeepSeekBalancePlugin::Instance();
}
