#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <variant>

#include <Windows.h>

struct RegAttr
{
  std::wstring name;
  uint32_t type;
  std::variant<std::wstring, uint32_t, uint64_t, std::vector<uint8_t>> value;
};

struct RegInfo
{
  LSTATUS retCode;
  HKEY hKey;
  SYSTEMTIME lasttime;
  std::vector<std::wstring> children;
  std::vector<RegAttr> attributes;
};

bool GetRegInfo(HKEY parent, LPCWSTR lpSubKey, RegInfo& info)
{
  info.retCode = RegOpenKeyExW(parent, lpSubKey, NULL, KEY_ALL_ACCESS, &info.hKey);

  if (info.retCode != ERROR_SUCCESS)
    return false;

  // 打开成功后枚举所有子项
  wchar_t className[MAX_PATH] = L""; // 类名缓冲区
  DWORD classNameSize = MAX_PATH;    // 类字符串的大小
  DWORD subKeys = 0;               // 子键数
  DWORD maxSubKey;                // 最长子密钥大小
  DWORD cchMaxClass;                // 最长的类串
  DWORD attrlen;                    // 键值的数量
  DWORD attrNameMaxLen;                // 最长值名称
  DWORD attrValueMaxLen;             // 最长值数据
  DWORD cbSecurityDescriptor;       // 安全描述符的大小
  FILETIME ftLastWriteTime;         // 上次写入时间

  // 检索(获取)有关指定注册表项的信息
  info.retCode = RegQueryInfoKeyW(
    info.hKey,                  // key handle
    className,              // 类名缓冲区
    &classNameSize,         // 类字符串的大小
    NULL,                  // 预订的

    &subKeys,             // 子键数
    &maxSubKey,          // 最长子密钥大小

    &cchMaxClass,          // 最长的类串
    &attrlen,              // 此键的值数
    &attrNameMaxLen,          // 最长值名称
    &attrValueMaxLen,       // 最长值数据
    &cbSecurityDescriptor, // 安全描述符
    &ftLastWriteTime       // 上次写入时间
  );
  FileTimeToSystemTime(&ftLastWriteTime, &info.lasttime);

  // 枚举子项
  if (subKeys)
  {
    maxSubKey += sizeof(char);          // 0结尾
    wchar_t* kname = new wchar_t[maxSubKey];  // 子键名的缓冲区
    DWORD knamesize;                     // 名称字符串的大小
    for (size_t i = 0; i < subKeys; ++i)
    {
      ZeroMemory(kname, maxSubKey);
      knamesize = maxSubKey;
      info.retCode = RegEnumKeyExW(info.hKey, i, kname, &knamesize, NULL, NULL, NULL, NULL);
      if (info.retCode == ERROR_SUCCESS)
        info.children.push_back(kname);
    }
    delete[] kname;
  }

  // 枚举属性
  if (attrlen)
  {
    wchar_t* name = new wchar_t[attrNameMaxLen];
    DWORD namesize;
    DWORD type = NULL;
    uint8_t* value = new uint8_t[attrValueMaxLen];
    DWORD valuesize;

    for (size_t i = 0; i < attrlen; ++i)
    {
      ZeroMemory(name, attrNameMaxLen);
      ZeroMemory(value, attrValueMaxLen);
      namesize = attrNameMaxLen;
      valuesize = attrValueMaxLen;

      info.retCode = RegEnumValueW(info.hKey, i, name, &namesize, NULL, &type, value, &valuesize);

      if (info.retCode == ERROR_SUCCESS)
      {
        RegAttr attr{ name, type };
        switch (type)
        {
        case REG_SZ:
        case REG_MULTI_SZ:
        case REG_EXPAND_SZ:
          attr.value = (wchar_t*)value;
          break;
        case REG_DWORD:
          attr.value = *(uint32_t*)value;
          break;
        case REG_QWORD:
          attr.value = *(uint64_t*)value;
          break;
        case REG_BINARY:
        {
          std::vector<uint8_t> bytes(valuesize);
          memcpy_s(bytes.data(), valuesize, value, valuesize);
          attr.value = bytes;
          break;
        }

        default:
          break;
        }
        info.attributes.push_back(attr);
      }

    }
    delete[] name;
    delete[] value;
  }

  return true;
}

std::wstring GetRegTypeString(uint32_t type) {
  switch (type)
  {
  case REG_SZ:
    return L"REG_SZ";
  case REG_MULTI_SZ:
    return L"REG_MULTI_SZ";
  case REG_EXPAND_SZ:
    return L"REG_EXPAND_SZ";
  case REG_DWORD:
    return L"REG_DWORD";
    break;
  case REG_QWORD:
    return L"REG_QWORD";
    break;
  case REG_BINARY:
    return L"REG_BINARY";
  default:
    return L"UNKNOW";
    break;
  }
}

std::wstring PrintHexValue(const std::vector<uint8_t>& bytes) {
  std::wstring r = L"hex:";

  for (auto x : bytes)
  {
    wchar_t buf[_MAX_ITOSTR_BASE16_COUNT];
    swprintf_s(buf, L"%02X", x);
    r.append(buf);
    r.push_back(',');
  }
  r.pop_back();
  return r;
}

void PrintReg(HKEY parent, LPCWSTR lpSubKey, std::wstring space = L"  ") {
  RegInfo info;
  if (GetRegInfo(parent, lpSubKey, info))
  {
    std::wcout << space << L"Key: " << lpSubKey << std::endl;

    // std::wcout << space << L"名称\t类型\t数据\t\n";
    for (auto a : info.attributes)
    {
      std::wcout << space
        << (a.name.empty() ? L"@" : a.name) << '=';

      switch (a.type)
      {
      case REG_SZ:
      case REG_MULTI_SZ:
      case REG_EXPAND_SZ:
        std::wcout << L"\"" << std::get<std::wstring>(a.value) << L"\"" << std::endl;
        break;
      case REG_DWORD:
        std::cout << "dword:" << std::get<uint32_t>(a.value) << std::endl;
        break;
      case REG_QWORD:
        std::cout << "qword:" << std::get<uint64_t>(a.value) << std::endl;
        break;
      case REG_BINARY:
        std::wcout << PrintHexValue(std::get<std::vector<uint8_t>>(a.value)) << std::endl;
        break;
      }
    }

    for (size_t i = 0; i < info.children.size(); i++)
    {
      auto s = info.children.at(i);
      PrintReg(info.hKey, s.data(), space + L"  ");
      std::cout << '\n';
    }

    RegCloseKey(info.hKey);
  }
}

int main()
{
  setlocale(LC_ALL, "chs");
   PrintReg(HKEY_CLASSES_ROOT, L"*\\shell");
  return 0;
}