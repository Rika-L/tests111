#include "Platform.h"
#include "OptionsDlg.h"
#include "DataManager.h"
#include "resource.h"
#include <commctrl.h>
#include <string>

PRAGMA_COMMENT_LIB("comctl32.lib")
PRAGMA_COMMENT_LIB("gdi32.lib")

// DLL instance handle from dllmain.cpp
extern HMODULE g_hInstance;

// Dialog procedure wrapper to route to class instance
static COptionsDlg* g_pOptionsDlg = nullptr;

COptionsDlg::COptionsDlg(HWND hParent)
    : m_hParent(hParent)
    , m_hDlg(nullptr)
    , m_hFont(nullptr)
{
}

COptionsDlg::~COptionsDlg()
{
    if (g_pOptionsDlg == this)
        g_pOptionsDlg = nullptr;
    if (m_hFont)
        DeleteObject(m_hFont);
}

INT_PTR COptionsDlg::DoModal()
{
    g_pOptionsDlg = this;
    return DialogBoxParamW(g_hInstance,
        MAKEINTRESOURCEW(IDD_OPTIONS_DIALOG),
        m_hParent, DlgProc, 0);
}

INT_PTR CALLBACK COptionsDlg::DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (!g_pOptionsDlg)
        return FALSE;
    switch (msg)
    {
    case WM_INITDIALOG:
        return g_pOptionsDlg->HandleInitDialog(hDlg);
    case WM_COMMAND:
        return g_pOptionsDlg->HandleCommand(hDlg, wParam, lParam);
    case WM_DESTROY:
        return g_pOptionsDlg->HandleDestroyDialog(hDlg);
    }
    return FALSE;
}

INT_PTR COptionsDlg::HandleInitDialog(HWND hDlg)
{
    m_hDlg = hDlg;
    const auto& config = CDataManager::Instance().GetConfig();

    // Set font FIRST so all subsequent text is rendered correctly.
    // .rc now uses UTF-16LE with Segoe UI, but explicitly setting the
    // font here provides a safety net for windres builds.
    m_hFont = CreateFontW(-9, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    if (m_hFont)
    {
        SendMessage(hDlg, WM_SETFONT, (WPARAM)m_hFont, MAKELPARAM(TRUE, 0));
        HWND hChild = GetWindow(hDlg, GW_CHILD);
        while (hChild)
        {
            SendMessage(hChild, WM_SETFONT, (WPARAM)m_hFont, MAKELPARAM(TRUE, 0));
            hChild = GetWindow(hChild, GW_HWNDNEXT);
        }
    }

    // Set saved values
    SetDlgItemTextW(hDlg, IDC_API_TOKEN_EDIT, config.api_token.c_str());
    SetDlgItemTextW(hDlg, IDC_API_URL_EDIT, config.api_url.c_str());
    SetDlgItemInt(hDlg, IDC_REFRESH_INTERVAL_EDIT, config.refresh_interval_sec / 60, FALSE);

    // Spin control range
    HWND hSpin = GetDlgItem(hDlg, IDC_REFRESH_INTERVAL_SPIN);
    if (hSpin)
    {
        SendMessage(hSpin, UDM_SETRANGE, 0, MAKELPARAM(60, 1));
        SendMessage(hSpin, UDM_SETPOS32, 0, config.refresh_interval_sec / 60);
    }

    // Chinese text is now set directly in .rc (UTF-16LE).
    // The calls below serve as fallback for any build toolchain that
    // doesn't handle the UTF-16LE .rc correctly (e.g. old windres).
    SetWindowTextW(hDlg, L"DeepSeek \u4F59\u989D\u63D2\u4EF6\u8BBE\u7F6E");
    SetDlgItemTextW(hDlg, IDC_API_SETTINGS_GROUP, L"API \u8BBE\u7F6E");
    SetDlgItemTextW(hDlg, IDC_REFRESH_SETTINGS_GROUP, L"\u5237\u65B0\u8BBE\u7F6E");

    return TRUE;
}

INT_PTR COptionsDlg::HandleCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(wParam))
    {
    case IDOK:
    {
        // Save settings
        wchar_t buff[1024];

        // API Token
        GetDlgItemTextW(hDlg, IDC_API_TOKEN_EDIT, buff, 1024);
        CDataManager::Instance().SetApiToken(buff);

        // API URL
        GetDlgItemTextW(hDlg, IDC_API_URL_EDIT, buff, 1024);
        if (wcslen(buff) > 0)
            CDataManager::Instance().SetApiUrl(buff);

        // Refresh interval (in minutes, convert to seconds)
        BOOL translated;
        int minutes = GetDlgItemInt(hDlg, IDC_REFRESH_INTERVAL_EDIT, &translated, FALSE);
        if (translated && minutes >= 1)
            CDataManager::Instance().SetRefreshInterval(minutes * 60);

        EndDialog(hDlg, IDOK);
        return TRUE;
    }

    case IDC_TEST_CONNECTION_BTN:
    {
        // Test the connection with current settings
        wchar_t buff[1024];
        GetDlgItemTextW(hDlg, IDC_API_TOKEN_EDIT, buff, 1024);

        if (wcslen(buff) == 0)
        {
            SetDlgItemTextW(hDlg, IDC_STATUS_TEXT, L"请输入 API Token");
            return TRUE;
        }

        // Save current edit fields to temp config for testing
        std::wstring saved_token = CDataManager::Instance().GetConfig().api_token;
        CDataManager::Instance().SetApiToken(buff);

        SetDlgItemTextW(hDlg, IDC_STATUS_TEXT, L"正在测试连接...");

        bool success = CDataManager::Instance().RefreshBalance();

        if (success)
        {
            const auto& data = CDataManager::Instance().GetBalanceData();
            if (data.has_data)
            {
                std::wstring msg = L"连接成功!\n余额: " +
                    data.balance_info.total_balance + L" " +
                    data.balance_info.currency;
                SetDlgItemTextW(hDlg, IDC_STATUS_TEXT, msg.c_str());
            }
            else
            {
                SetDlgItemTextW(hDlg, IDC_STATUS_TEXT, L"连接成功，但解析响应失败");
            }
        }
        else
        {
            SetDlgItemTextW(hDlg, IDC_STATUS_TEXT, L"连接失败，请检查 Token 和网络连接");
        }
        return TRUE;
    }

    case IDCANCEL:
        EndDialog(hDlg, IDCANCEL);
        return TRUE;
    }
    return FALSE;
}

INT_PTR COptionsDlg::HandleDestroyDialog(HWND /*hDlg*/)
{
    if (m_hFont)
    {
        DeleteObject(m_hFont);
        m_hFont = nullptr;
    }
    return FALSE;
}
