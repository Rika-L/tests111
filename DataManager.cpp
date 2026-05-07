#include "Platform.h"
#include "DataManager.h"
#include <string>
#include <vector>
#include <winhttp.h>
#include <windows.h>

PRAGMA_COMMENT_LIB("winhttp.lib")

CDataManager CDataManager::m_instance;

CDataManager::CDataManager()
{
}

CDataManager::~CDataManager()
{
    SaveConfig();
}

CDataManager& CDataManager::Instance()
{
    return m_instance;
}

void CDataManager::LoadConfig(const std::wstring& config_dir)
{
    // Build config path from DLL path + ".ini"
    wchar_t path[MAX_PATH];
    HMODULE hModule;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
        reinterpret_cast<LPCWSTR>(&CDataManager::LoadConfig), &hModule);
    GetModuleFileNameW(hModule, path, MAX_PATH);
    std::wstring module_path = path;
    m_config_path = module_path;
    if (!config_dir.empty())
    {
        size_t index = module_path.find_last_of(L"\\/");
        std::wstring module_file_name = module_path.substr(index + 1);
        m_config_path = config_dir + module_file_name;
    }
    m_config_path += L".ini";

    // Read config from INI file
    wchar_t buff[1024];

    // API Token
    GetPrivateProfileStringW(L"config", L"api_token", L"", buff, 1024, m_config_path.c_str());
    m_config.api_token = buff;

    // API URL
    GetPrivateProfileStringW(L"config", L"api_url",
        L"https://api.deepseek.com/user/balance", buff, 1024, m_config_path.c_str());
    m_config.api_url = buff;

    // Refresh interval (seconds)
    m_config.refresh_interval_sec = GetPrivateProfileIntW(L"config",
        L"refresh_interval", 1800, m_config_path.c_str());
    if (m_config.refresh_interval_sec < 30)
        m_config.refresh_interval_sec = 30; // minimum 30 seconds
}

void CDataManager::SaveConfig() const
{
    WritePrivateProfileStringW(L"config", L"api_token", m_config.api_token.c_str(), m_config_path.c_str());
    WritePrivateProfileStringW(L"config", L"api_url", m_config.api_url.c_str(), m_config_path.c_str());

    wchar_t buff[16];
    StringPrintf(buff, L"%d", m_config.refresh_interval_sec);
    WritePrivateProfileStringW(L"config", L"refresh_interval", buff, m_config_path.c_str());
}

void CDataManager::SetApiToken(const std::wstring& token)
{
    m_config.api_token = token;
    SaveConfig();
}

void CDataManager::SetRefreshInterval(int seconds)
{
    if (seconds < 30) seconds = 30;
    m_config.refresh_interval_sec = seconds;
    SaveConfig();
}

void CDataManager::SetApiUrl(const std::wstring& url)
{
    m_config.api_url = url;
    SaveConfig();
}

bool CDataManager::HttpGetJson(const std::wstring& url, const std::wstring& token, std::string& response)
{
    // Parse URL to get host and path
    URL_COMPONENTSW url_comp = { 0 };
    url_comp.dwStructSize = sizeof(url_comp);
    url_comp.dwHostNameLength = 1; // non-zero to force parsing
    url_comp.dwUrlPathLength = 1;
    url_comp.dwSchemeLength = 1;

    if (!WinHttpCrackUrlW(url.c_str(), (DWORD)url.length(), 0, &url_comp))
        return false;

    std::wstring host_name(url_comp.lpszHostName, url_comp.dwHostNameLength);
    std::wstring url_path(url_comp.lpszUrlPath, url_comp.dwUrlPathLength);
    if (url_comp.dwExtraInfoLength > 0)
    {
        url_path += std::wstring(url_comp.lpszExtraInfo, url_comp.dwExtraInfoLength);
    }

    BOOL bResults = FALSE;
    HINTERNET hSession = nullptr, hConnect = nullptr, hRequest = nullptr;

    hSession = WinHttpOpen(L"DeepSeekBalancePlugin/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, nullptr, nullptr, 0);

    if (!hSession)
        return false;

    INTERNET_PORT port = (url_comp.nScheme == INTERNET_SCHEME_HTTPS) ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;

    hConnect = WinHttpConnect(hSession, host_name.c_str(), port, 0);
    if (!hConnect)
    {
        WinHttpCloseHandle(hSession);
        return false;
    }

    DWORD flags = (url_comp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
    hRequest = WinHttpOpenRequest(hConnect, L"GET", url_path.c_str(), nullptr, nullptr, nullptr, flags);
    if (!hRequest)
    {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    // Set auth header
    std::wstring auth_header = L"Authorization: Bearer " + token;
    WinHttpAddRequestHeaders(hRequest, auth_header.c_str(), (DWORD)auth_header.length(), WINHTTP_ADDREQ_FLAG_ADD);

    // Set Accept header
    WinHttpAddRequestHeaders(hRequest, L"Accept: application/json", (DWORD)-1, WINHTTP_ADDREQ_FLAG_ADD);

    bResults = WinHttpSendRequest(hRequest, nullptr, 0, nullptr, 0, 0, 0);
    if (!bResults)
    {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    bResults = WinHttpReceiveResponse(hRequest, nullptr);
    if (!bResults)
    {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    // Read response
    DWORD dwSize = 0;
    response.clear();

    do
    {
        dwSize = 0;
        if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
            break;

        if (dwSize == 0)
            break;

        std::vector<char> buffer(dwSize + 1, 0);
        DWORD dwDownloaded = 0;

        if (!WinHttpReadData(hRequest, buffer.data(), dwSize, &dwDownloaded))
            break;

        response.append(buffer.data(), dwDownloaded);
    } while (dwSize > 0);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return !response.empty();
}

bool CDataManager::ParseBalanceResponse(const std::string& json, BalanceData& data)
{
    // Simple JSON parsing without external dependencies
    // Expected format:
    // {"is_available": true, "balance_infos": [{"currency": "CNY", "total_balance": "8.87", ...}]}

    data.has_data = false;

    // Parse is_available
    auto avail_pos = json.find("\"is_available\"");
    if (avail_pos == std::string::npos)
        return false;

    auto colon_pos = json.find(':', avail_pos + 13);
    if (colon_pos == std::string::npos)
        return false;

    auto val_start = json.find_first_not_of(" \t", colon_pos + 1);
    if (val_start == std::string::npos)
        return false;

    data.is_available = (json.compare(val_start, 4, "true") == 0);

    // Find balance_infos array
    auto infos_pos = json.find("\"balance_infos\"");
    if (infos_pos == std::string::npos)
        return false;

    auto array_start = json.find('[', infos_pos);
    if (array_start == std::string::npos)
        return false;

    auto obj_start = json.find('{', array_start);
    if (obj_start == std::string::npos)
        return false;

    auto obj_end = json.find('}', obj_start);
    if (obj_end == std::string::npos)
        return false;

    std::string obj = json.substr(obj_start, obj_end - obj_start + 1);

    // Helper lambda to extract string value by key
    auto extract_str = [&](const std::string& key) -> std::string {
        auto key_pos = obj.find("\"" + key + "\"");
        if (key_pos == std::string::npos) return "";

        auto col_pos = obj.find(':', key_pos + key.length() + 2);
        if (col_pos == std::string::npos) return "";

        auto quote_start = obj.find('"', col_pos + 1);
        if (quote_start == std::string::npos) return "";

        auto quote_end = obj.find('"', quote_start + 1);
        if (quote_end == std::string::npos) return "";

        return obj.substr(quote_start + 1, quote_end - quote_start - 1);
    };

    std::string currency = extract_str("currency");
    std::string total = extract_str("total_balance");
    std::string granted = extract_str("granted_balance");
    std::string topped_up = extract_str("topped_up_balance");

    // Convert to wide strings
    auto to_wide = [](const std::string& s) -> std::wstring {
        if (s.empty()) return L"";
        int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
        std::wstring result(len, 0);
        MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &result[0], len);
        return result;
    };

    data.balance_info.currency = to_wide(currency);
    data.balance_info.total_balance = to_wide(total);
    data.balance_info.granted_balance = to_wide(granted);
    data.balance_info.topped_up_balance = to_wide(topped_up);
    data.has_data = true;

    return true;
}

bool CDataManager::RefreshBalance()
{
    if (m_config.api_token.empty())
        return false;

    std::string json_response;
    if (!HttpGetJson(m_config.api_url, m_config.api_token, json_response))
        return false;

    BalanceData new_data;
    if (!ParseBalanceResponse(json_response, new_data))
        return false;

    m_balance_data = new_data;

    // Update timestamp
    SYSTEMTIME st;
    GetLocalTime(&st);
    wchar_t buff[64];
    StringPrintf(buff, L"%.2d:%.2d:%.2d", st.wHour, st.wMinute, st.wSecond);
    m_balance_data.last_update_time = buff;
    m_last_refresh_time = GetTickCount64();

    return true;
}
