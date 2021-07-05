#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

#include <Windows.h>

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

int main()
{
    // 打开指定的注册表项,键名不区分大小写
    HKEY hKey;
    LSTATUS retCode;

    retCode = RegOpenKeyExW(HKEY_CLASSES_ROOT, L"*\\shell", NULL, KEY_ALL_ACCESS, &hKey);
    if (retCode != ERROR_SUCCESS)
    {
        if (retCode == ERROR_FILE_NOT_FOUND)
            printf("Key not found.\n");
        else
            printf("Error opening key.\n");
        return 1;
    }

    // 打开成功后枚举所有子项
    wchar_t achKey[MAX_KEY_LENGTH];   // 子键名的缓冲区
    DWORD cbName;                     // 名称字符串的大小
    wchar_t achClass[MAX_PATH] = L""; // 类名缓冲区
    DWORD cchClassName = MAX_PATH;    // 类字符串的大小
    DWORD cSubKeys = 0;               // 子键数
    DWORD cbMaxSubKey;                // 最长子密钥大小
    DWORD cchMaxClass;                // 最长的类串
    DWORD cValues;                    // 键值的数量
    DWORD cchMaxValue;                // 最长值名称
    DWORD cbMaxValueData;             // 最长值数据
    DWORD cbSecurityDescriptor;       // 安全描述符的大小
    FILETIME ftLastWriteTime;         // 上次写入时间

    SYSTEMTIME st; // 上次写入时间

    // 检索(获取)有关指定注册表项的信息
    retCode = RegQueryInfoKeyW(
        hKey,                  // key handle
        achClass,              // 类名缓冲区
        &cchClassName,         // 类字符串的大小
        NULL,                  // 预订的
        &cSubKeys,             // 子键数
        &cbMaxSubKey,          // 最长子密钥name大小
        &cchMaxClass,          // 最长的类串
        &cValues,              // 此键的值数
        &cchMaxValue,          // 最长值名称
        &cbMaxValueData,       // 最长值数据
        &cbSecurityDescriptor, // 安全描述符
        &ftLastWriteTime       // 上次写入时间
    );

    // 枚举子项，直到 RegEnumKeyEx 失败
    if (cSubKeys)
    {
        printf("\nNumber of subkeys: %d\n", cSubKeys);
        for (size_t i = 0; i < cSubKeys; ++i)
        {
            cbName = MAX_KEY_LENGTH;
            retCode = RegEnumKeyExW(hKey,
                                    i,
                                    achKey,
                                    &cbName,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &ftLastWriteTime);
            if (retCode == ERROR_SUCCESS)
            {
                FileTimeToSystemTime(&ftLastWriteTime, &st);
                printf("(%d) {name:%ls,time:%i-%i-%i}\n", i + 1, achKey, st.wYear, st.wMonth, st.wDay);
            }
        }
    }

    // 枚举键值 (右侧属性)
    if (cValues)
    {
        printf("\nNumber of values: %d\n", cValues);
        wchar_t achValue[MAX_VALUE_NAME]; // 名称
        DWORD cchValue = MAX_VALUE_NAME;  // 名称缓冲区大小
        DWORD type = NULL;                // 类型
        LPBYTE data;                      // data
        DWORD cbData;                     // data缓冲区大小

        for (size_t i = 0; i < cValues; ++i)
        {
            // 为数据分配缓冲区的最佳方式
            cchValue = MAX_VALUE_NAME;
            achValue[0] = '\0';
            retCode = RegEnumValueW(hKey,
                                    i,
                                    achValue,
                                    &cchValue,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &cbData);

            cchValue += sizeof(wchar_t); // 加上终止空字符

            if (retCode != ERROR_SUCCESS)
                continue;

            // 获取数据
            data = new BYTE[cbData];
            retCode = RegEnumValueW(hKey,
                                    i,
                                    achValue,
                                    &cchValue,
                                    NULL, // 此参数是保留的，必须为NULL
                                    &type,
                                    data,
                                    &cbData);

            if (achValue[0] == '\0')
                wcscpy_s(achValue, MAX_VALUE_NAME, L"Default");

            if (retCode == ERROR_SUCCESS)
            {
                switch (type)
                {
                case REG_SZ:
                    printf("(%d) {name:%ls,type:%s,data:%ls}\n", i + 1, achValue, "REG_SZ", (wchar_t *)data);
                    break;
                case REG_DWORD:
                    printf("(%d) {name:%ls,type:%s,data: %d}\n", i + 1, achValue, "REG_DWORD", *(DWORD *)data);
                    break;
                case REG_QWORD:
                    printf("(%d) {name:%ls,type:%s,data: %d}\n", i + 1, achValue, "REG_QWORD", *(uint64_t *)data);
                    break;
                case REG_BINARY:
                {
                    std::string str;
                    for (size_t i = 0; i < cbData; i++)
                    {
                        size_t n = (size_t)(*(data + i));
                        std::stringstream sstream;
                        sstream << std::setfill('0') << std::setw(sizeof(BYTE) * 2)
                                << std::uppercase << std::hex << n;
                        str += sstream.str();
                        str.push_back(' ');
                    }
                    str.push_back(0);
                    printf("(%d) {name:%ls,type:%s,data:%s}\n", i + 1, achValue, "REG_BINARY", str.data());
                    break;
                }

                default:
                    break;
                }
            }

            delete[] data;
        }
    }

    // 关闭指定注册表项的句柄
    retCode = RegCloseKey(hKey);
    if (retCode != ERROR_SUCCESS)
    {
        printf("Error close hkey.\n");
        return 1;
    }
    return 0;
}