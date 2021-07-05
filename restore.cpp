#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

#include <Windows.h>

#include "ModifyPrivilege.hpp"

int main()
{
    //! 先备份所有注册表
    // 读取指定文件中的注册表信息并将其复制到指定的键上

    // 获取权限
    if (!SUCCEEDED(ModifyPrivilege(SE_RESTORE_NAME, true)))
        return 1;
    if (!SUCCEEDED(ModifyPrivilege(SE_BACKUP_NAME, true)))
        return 1;

    HKEY hKey;
    LSTATUS retCode;
    retCode = RegCreateKeyExW(
        HKEY_CLASSES_ROOT,
        L"*\\shell\\ffplay",
        NULL,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS,
        NULL,
        &hKey,
        NULL);

    if (retCode != ERROR_SUCCESS)
    {
        printf("Error Create Key.\n");
        return 1;
    }

    retCode = RegRestoreKeyW(
        hKey,
        L"C:\\Users\\ajanuw\\Desktop\\save.txt", // 使用RegSaveKey函数创建的文件
        REG_FORCE_RESTORE);

    if (retCode != ERROR_SUCCESS)
    {
        printf("Error Restore key (%d)\n", retCode);
        return 1;
    }

    if (RegCloseKey(hKey) != ERROR_SUCCESS)
    {
        printf("Error close.\n");
        return 1;
    }
    return 0;
}