#pragma once
// Main handler for account and its information.

class CAccount
{
public:
	std::string m_sName;
	std::string m_sAlternativeName = "";
	std::string m_sLastSwitchDate = "Unk";

	CAccount() { }
};

namespace AccountManager
{
	const char* m_pDirectoryName = "accounts";
	const char* m_pVDFName = "config.vdf";
	const char* m_pInfoName = "info.dat";

	std::string GetPath(std::string m_sAccountName)
	{
		return std::string(m_pDirectoryName) + "\\" + m_sAccountName;
	}

	std::string GetSSFNPath(std::string m_sAccountName)
	{
		return GetPath(m_sAccountName) + "\\ssfn";
	}

	std::vector<CAccount> m_vAccounts;
	std::string GetLastSwitchDate(std::string m_sAccountName)
	{
		for (CAccount& Account : m_vAccounts)
		{
			if (Account.m_sName == m_sAccountName)
				return Account.m_sLastSwitchDate;
		}

		return "Unk";
	}

	void TryAdd(std::string m_sAccountName)
	{
		std::string m_sPath = GetPath(m_sAccountName);

		FILE* m_pFile = nullptr;
		std::string m_sVDFPath = m_sPath + "\\" + m_pVDFName;
		if (fopen_s(&m_pFile, &m_sVDFPath[0], "r"))
		{
			remove(&m_sPath[0]); // Couldn't find VDF file we take this as fault account and remove it.
			return;
		}
		else
		{
			fclose(m_pFile);
			m_pFile = nullptr;
		}

		CAccount Account;
		Account.m_sName = m_sAccountName;

		std::string m_sInfoPath = m_sPath + "\\" + m_pInfoName;
		if (!fopen_s(&m_pFile, &m_sInfoPath[0], "r"))
		{
			// Parse info
			{
				char m_cLine[128];

				int m_iLine = 0;
				while (fgets(m_cLine, sizeof(m_cLine), m_pFile))
				{
					m_cLine[strcspn(m_cLine, "\n")] = 0;

					switch (m_iLine)
					{
						default: break;
						case 0: Account.m_sAlternativeName = m_cLine; break;
						case 1: Account.m_sLastSwitchDate = m_cLine; break;
					}

					m_iLine++;
				}
			}
			fclose(m_pFile);
		}

		m_vAccounts.push_back(Account);
	}

	void WriteInfo(std::string m_sAccountName, std::string m_sAlternativeName = "", std::string m_sLastSwitch = "")
	{
		FILE* m_pFile = nullptr;
		std::string m_sInfoPath = GetPath(m_sAccountName) + "\\" + m_pInfoName;

		if (!fopen_s(&m_pFile, &m_sInfoPath[0], "w"))
		{
			fprintf(m_pFile, "%s\n", &m_sAlternativeName[0]);
			fprintf(m_pFile, "%s", m_sLastSwitch.empty() ? &GetLastSwitchDate(m_sAccountName)[0] : &m_sLastSwitch[0]);

			fclose(m_pFile);
		}
	}

	void New(std::string m_sAccountName, std::string m_sPathToVDF)
	{
		std::string m_sPath = GetPath(m_sAccountName);
		CreateDirectoryA(&m_sPath[0], 0);

		std::string m_sVDFPath = m_sPath + "\\" + m_pVDFName;
		CopyFileA(&m_sPathToVDF[0], &m_sVDFPath[0], FALSE);

		WriteInfo(m_sAccountName);
	}

	void Refresh()
	{
		m_vAccounts.clear();

		std::string m_sPath = std::string(m_pDirectoryName) + "\\*";
		WIN32_FIND_DATA m_wFindData;

		HANDLE m_hFind = FindFirstFileA(&m_sPath[0], &m_wFindData);
		if (m_hFind)
		{
			while (FindNextFileA(m_hFind, &m_wFindData))
			{
				if (m_wFindData.cFileName[0] == '.') continue;

				TryAdd(m_wFindData.cFileName);
			}

			FindClose(m_hFind);
		}
	}

	void Init()
	{
		CreateDirectoryA(m_pDirectoryName, 0);

		Refresh();
	}
}