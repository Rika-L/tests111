#pragma once
#include <windows.h>

class COptionsDlg
{
public:
    COptionsDlg(HWND hParent);
    ~COptionsDlg();

    INT_PTR DoModal();

private:
    static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
    INT_PTR HandleInitDialog(HWND hDlg);
    INT_PTR HandleCommand(HWND hDlg, WPARAM wParam, LPARAM lParam);

    HWND m_hParent;
    HWND m_hDlg;
};
