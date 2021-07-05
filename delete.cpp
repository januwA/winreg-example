#include <iostream>
#include <Windows.h>

int main()
{
    // 从注册表的指定平台特定视图中删除子项及其值
    // 递归删除
    if (RegDeleteTreeW(HKEY_CLASSES_ROOT, L"*\\shell\\ffplay") != ERROR_SUCCESS)
    {
        printf("RegDeleteKeyExW Fail %d\n", GetLastError());
        return 1;
    }
    return 0;
}