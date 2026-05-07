#pragma once
#include "PluginInterface.h"
#include "DeepSeekBalanceItem.h"
#include "DataManager.h"

class CDeepSeekBalancePlugin : public ITMPlugin
{
private:
    CDeepSeekBalancePlugin();

public:
    static CDeepSeekBalancePlugin& Instance();

    // ITMPlugin
    virtual IPluginItem* GetItem(int index) override;
    virtual void DataRequired() override;
    virtual const wchar_t* GetInfo(PluginInfoIndex index) override;
    virtual OptionReturn ShowOptionsDialog(void* hParent) override;
    virtual const wchar_t* GetTooltipInfo() override;
    virtual void OnExtenedInfo(ExtendedInfoIndex index, const wchar_t* data) override;

private:
    CDeepSeekBalanceItem m_balance_item;
    ITrafficMonitor* m_app{};

    static CDeepSeekBalancePlugin m_instance;
};

#ifdef __cplusplus
extern "C" {
#endif
    __declspec(dllexport) ITMPlugin* TMPluginGetInstance();
#ifdef __cplusplus
}
#endif
