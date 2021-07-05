#pragma once

#include <iostream>
#include <Windows.h>

HRESULT ModifyPrivilege(
    IN LPCTSTR szPrivilege,
    IN BOOL fEnable)
{
    HRESULT hr = S_OK;
    TOKEN_PRIVILEGES NewState;
    LUID luid;
    HANDLE hToken = NULL;

    // 打开此进程的进程令牌
    if (!OpenProcessToken(GetCurrentProcess(),
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                          &hToken))
    {
        printf("Failed OpenProcessToken\n");
        return ERROR_FUNCTION_FAILED;
    }

    // 获取权限的本地唯一 ID
    if (!LookupPrivilegeValue(NULL,
                              szPrivilege,
                              &luid))
    {
        CloseHandle(hToken);
        printf("Failed LookupPrivilegeValue\n");
        return ERROR_FUNCTION_FAILED;
    }

    // 为 TOKEN_PRIVILEGE 结构赋值
    NewState.PrivilegeCount = 1;
    NewState.Privileges[0].Luid = luid;
    NewState.Privileges[0].Attributes =
        (fEnable ? SE_PRIVILEGE_ENABLED : 0);

    // 调整令牌权限
    if (!AdjustTokenPrivileges(hToken,
                               FALSE,
                               &NewState,
                               0,
                               NULL,
                               NULL))
    {
        printf("Failed AdjustTokenPrivileges\n");
        hr = ERROR_FUNCTION_FAILED;
    }

    CloseHandle(hToken);
    return hr;
}
