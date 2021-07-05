#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

#include <Windows.h>

HKEY create(HKEY parent, LPCWSTR lpSubKey)
{
    HKEY hKey;

    // 如果Key已经存在，需要编辑可以使用 RegSetKeyValueW
    // 删除使用 RegDeleteKeyExW
    LSTATUS retCode = RegCreateKeyExW(
        parent,
        lpSubKey,
        NULL,
        NULL,
        REG_OPTION_NON_VOLATILE, // 此密钥不是易失性的,重启系统后依旧存在
        KEY_ALL_ACCESS,
        NULL,
        &hKey,
        NULL);
    if (retCode != ERROR_SUCCESS)
    {
        printf("Error RegCreateKeyExW %d\n", retCode);
        return NULL;
    }
    return hKey;
}

bool setValue(HKEY hKey, const wchar_t *lpData, LPCWSTR lpValueName = nullptr)
{
    // 删除使用 RegDeleteValueW
    LSTATUS retCode = RegSetValueExW(
        hKey,
        lpValueName,
        NULL,
        REG_SZ,
        (BYTE *)lpData,
        wcsnlen(lpData, 1024) * sizeof(wchar_t));
    if (retCode != ERROR_SUCCESS)
    {
        printf("Error RegSetValueExW (%d)\n", retCode);
        return true;
    }

    return false;
}

int main()
{
    // 创建指定的注册表项
    LSTATUS retCode;

    // HKEY_CLASSES_ROOT\*\shell\ffplay
    HKEY hKey = create(HKEY_CLASSES_ROOT, L"*\\shell\\ffplay");
    if (!hKey)
        return 1;

    // HKEY_CLASSES_ROOT\*\shell\ffplay\command
    HKEY hKey2 = create(hKey, L"command");
    if (!hKey2)
        return 1;

    // 为创建的Key设置属性
    if (setValue(hKey, L"ffplay"))
        return 1;

    if (setValue(hKey2, L"\"D:\\dev\\tool\\ffmpeg\\bin\\ffplay.exe\" \"%1\""))
        return 1;

    // 关闭指定注册表项的句柄
    if (RegCloseKey(hKey) != ERROR_SUCCESS)
    {
        printf("Error close hkey.\n");
        return 1;
    }
    if (RegCloseKey(hKey2) != ERROR_SUCCESS)
    {
        printf("Error close hkey2.\n");
        return 1;
    }
    return 0;
}