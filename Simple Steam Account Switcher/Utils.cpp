#include "Includes.hpp"

namespace Utils
{
	unsigned int GetProcessID(const char* m_pName)
	{
		unsigned int m_uProcessID = 0U;

		HANDLE m_hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (m_hSnapshot)
		{
			PROCESSENTRY32 m_pEntry32;
			m_pEntry32.dwSize = sizeof(PROCESSENTRY32);
			if (Process32First(m_hSnapshot, &m_pEntry32))
			{
				while (Process32Next(m_hSnapshot, &m_pEntry32))
				{
					if (strstr(m_pEntry32.szExeFile, m_pName))
					{
						m_uProcessID = m_pEntry32.th32ProcessID;
						break;
					}
				}
			}

			CloseHandle(m_hSnapshot);
		}

		return m_uProcessID;
	}

	namespace EnumWindows
	{
		std::vector<HWND> m_vList;

		BOOL __stdcall GetListForWindowClass(HWND m_hWindow, LPARAM m_pParam)
		{
			char m_cClassName[128];
			GetClassNameA(m_hWindow, m_cClassName, sizeof(m_cClassName));

			if (strcmp(m_cClassName, reinterpret_cast<const char*>(m_pParam)) == 0)
				m_vList.emplace_back(m_hWindow);

			return TRUE;
		}
	}

	namespace Registry
	{
		unsigned int GetDWORD(HKEY m_hKey, const char* m_pPath, const char* m_pKey)
		{
			unsigned int m_uReturn = 0U;

			HKEY m_hRegistryKey;
			if (RegOpenKeyA(m_hKey, m_pPath, &m_hRegistryKey) == ERROR_SUCCESS)
			{
				DWORD m_dDataSize = sizeof(m_uReturn);
				RegQueryValueExA(m_hRegistryKey, m_pKey, 0, 0, LPBYTE(&m_uReturn), &m_dDataSize);

				RegCloseKey(m_hRegistryKey);
			}

			return m_uReturn;
		}

		std::string GetString(HKEY m_hKey, const char* m_pPath, const char* m_pKey)
		{
			std::string m_sReturn(128, '\0');

			HKEY m_hRegistryKey;
			if (RegOpenKeyA(m_hKey, m_pPath, &m_hRegistryKey) == ERROR_SUCCESS)
			{
				DWORD m_dDataSize = m_sReturn.size();
				if (RegQueryValueExA(m_hRegistryKey, m_pKey, 0, 0, LPBYTE(&m_sReturn[0]), &m_dDataSize) == ERROR_SUCCESS)
					m_sReturn.resize(m_dDataSize - 1);
				else
					m_sReturn.clear();

				RegCloseKey(m_hRegistryKey);
			}

			return m_sReturn;
		}

		bool WriteDWORD(HKEY m_hKey, const char* m_pPath, const char* m_pKey, unsigned int m_uValue)
		{
			bool m_bSuccess = false;

			HKEY m_hRegistryKey;
			if (RegOpenKeyA(m_hKey, m_pPath, &m_hRegistryKey) == ERROR_SUCCESS)
			{
				if (RegSetValueExA(m_hRegistryKey, m_pKey, 0, REG_DWORD, LPBYTE(&m_uValue), sizeof(m_uValue)) == ERROR_SUCCESS)
					m_bSuccess = true;

				RegCloseKey(m_hRegistryKey);
			}

			return m_bSuccess;
		}

		bool WriteString(HKEY m_hKey, const char* m_pPath, const char* m_pKey, std::string m_sValue)
		{
			bool m_bSuccess = false;

			HKEY m_hRegistryKey;
			if (RegOpenKeyA(m_hKey, m_pPath, &m_hRegistryKey) == ERROR_SUCCESS)
			{
				if (RegSetValueExA(m_hRegistryKey, m_pKey, 0, REG_SZ, LPBYTE(&m_sValue[0]), m_sValue.size() + 1) == ERROR_SUCCESS)
					m_bSuccess = true;

				RegCloseKey(m_hRegistryKey);
			}

			return m_bSuccess;
		}
	}
}