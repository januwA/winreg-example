#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

#include <Windows.h>

#include "ModifyPrivilege.hpp"

int main()
{
    // 备份注册表信息

    // 获取权限
    if (!SUCCEEDED(ModifyPrivilege(SE_BACKUP_NAME, true)))
        return 1;

    // 保存
    HKEY hKey;
    LSTATUS retCode;

    retCode = RegOpenKeyExW(HKEY_CLASSES_ROOT, L"*\\shell\\ffplay", NULL, KEY_ALL_ACCESS, &hKey);
    if (retCode != ERROR_SUCCESS)
    {
        if (retCode == ERROR_FILE_NOT_FOUND)
            printf("Key not found.\n");
        else
            printf("Error opening key.\n");
        return 1;
    }

    // 保存的是二进制文件
    retCode = RegSaveKeyW(
        hKey,
        L"C:\\Users\\ajanuw\\Desktop\\save.txt",
        NULL);

    if (retCode != ERROR_SUCCESS)
    {
        printf("Error save key (%d)\n", retCode);
        return 1;
    }

    if (RegCloseKey(hKey) != ERROR_SUCCESS)
    {
        printf("Error close.\n");
        return 1;
    }
    return 0;
}