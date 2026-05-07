#pragma once
#include <string>
#include <windows.h>

// Balance data from DeepSeek API
struct BalanceInfo
{
    std::wstring currency;          // e.g. "CNY"
    std::wstring total_balance;     // e.g. "8.87"
    std::wstring granted_balance;   // e.g. "0.00"
    std::wstring topped_up_balance; // e.g. "8.87"
};

struct BalanceData
{
    bool is_available{ false };
    BalanceInfo balance_info;
    std::wstring last_update_time;  // timestamp of last successful update
    bool has_data{ false };         // whether we have any data at all
};

struct PluginConfig
{
    std::wstring api_token;
    std::wstring api_url;           // customizable API URL
    int refresh_interval_sec{ 1800 }; // default: 30 minutes
};

class CDataManager
{
private:
    CDataManager();
    ~CDataManager();

public:
    static CDataManager& Instance();

    void LoadConfig(const std::wstring& config_dir);
    void SaveConfig() const;

    // HTTP request to DeepSeek API
    bool RefreshBalance();

    // Getters
    const BalanceData& GetBalanceData() const { return m_balance_data; }
    const PluginConfig& GetConfig() const { return m_config; }
    void SetApiToken(const std::wstring& token);
    void SetRefreshInterval(int seconds);
    void SetApiUrl(const std::wstring& url);
    std::wstring GetConfigPath() const { return m_config_path; }
    long long GetLastRefreshTime() const { return m_last_refresh_time; }

private:
    bool HttpGetJson(const std::wstring& url, const std::wstring& token, std::string& response);
    bool ParseBalanceResponse(const std::string& json, BalanceData& data);

    static CDataManager m_instance;

    PluginConfig m_config;
    BalanceData m_balance_data;
    std::wstring m_config_path;
    long long m_last_refresh_time{ 0 };
};
