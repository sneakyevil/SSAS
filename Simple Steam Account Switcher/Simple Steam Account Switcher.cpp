#include "Includes.hpp"
#include "AccountManager.hpp"
#include "Console.hpp"

#define ENABLE_SSFN 1

namespace Program
{
    void Info()
    {
        Console::Clear();
        Console::Print(Console::m_uDefaultColor, '!', "SSAS v1.0 made by "); Console::Print(CLR_BRED, "sneakyevil\n");
        Console::Print(Console::m_uDefaultColor, '!', "Loaded accounts: ");  Console::Print(CLR_BYELLOW, std::to_string(AccountManager::m_vAccounts.size()) + "\n\n");
    }

    int Error(std::string m_sError)
    {
        Info();
        Console::Print(CLR_BRED, '!', m_sError); _getch();
        return EXIT_FAILURE;
    }

    int GetSelectionIndex(std::string m_sText, std::string* m_pList, int m_iSize, int m_iDeltaShow, std::string m_sPrefix)
    {
        int m_iIndex = 0;
        bool m_bUpdate = true;

        while (1)
        {
            if (m_bUpdate)
            {
                if (0 > m_iDeltaShow || m_iDeltaShow >= m_iSize) m_iDeltaShow = m_iSize - 1;

                if (0 > m_iIndex) m_iIndex = m_iSize - 1;
                else if (m_iIndex >= m_iSize) m_iIndex = 0;

                m_bUpdate = false;

                Program::Info();
                Console::Print(Console::m_uDefaultColor, '!', m_sText);
                Console::DrawSelection(m_iIndex, m_pList, m_iSize, 2, "\t");
            }

            int m_iKey = _getch();
            switch (m_iKey)
            {
                case 8: return -1;
                case 13: return m_iIndex;
                case 72: 
                {
                    m_iIndex--; 
                    m_bUpdate = true;
                }
                break;
                case 80:
                {
                    m_iIndex++; 
                    m_bUpdate = true;
                }
                break;
            }
        }
    }

    namespace Steam
    {
        std::string m_sPath = "";
        std::string m_sVDFPath = "";
        std::string m_sExecutable = "steam.exe";
        const char* m_pSteamRegistry = "Software\\Valve\\Steam";

        bool Init()
        {
            m_sPath = Utils::Registry::GetString(HKEY_CURRENT_USER, m_pSteamRegistry, "SteamPath");
            m_sVDFPath = m_sPath + "\\config\\loginusers.vdf";

            return !m_sPath.empty();
        }

        std::string GetLoginUser() { return Utils::Registry::GetString(HKEY_CURRENT_USER, m_pSteamRegistry, "AutoLoginUser"); }

        bool IsRememberPasswordChecked() { return (Utils::Registry::GetDWORD(HKEY_CURRENT_USER, m_pSteamRegistry, "RememberPassword") == 1U); }

        bool IsRunning() { return (Utils::GetProcessID(&m_sExecutable[0]) != 0U); }

        void Start(std::string m_sArgs = "")
        {
            std::string m_sSteamExe = m_sPath + "\\" + m_sExecutable + " " + m_sArgs;

            STARTUPINFO m_sStartupInfo;
            ZeroMemory(&m_sStartupInfo, sizeof(m_sStartupInfo));
            m_sStartupInfo.cb = sizeof(STARTUPINFO);

            PROCESS_INFORMATION m_pProcessInfo;

            if (!CreateProcessA(0, &m_sSteamExe[0], 0, 0, 0, 0, 0, 0, &m_sStartupInfo, &m_pProcessInfo))
                return;

            CloseHandle(m_pProcessInfo.hProcess);
            CloseHandle(m_pProcessInfo.hThread);
        }

        void Exit()
        {
            if (!IsRunning()) return;

            Start("-exitsteam");

            while (IsRunning())
                Sleep(500);
        }
    }

    std::vector<std::string> GetFormattedAccountList()
    {
        std::vector<std::string> m_vReturn;

        int m_iCount = 1;
        for (CAccount& Account : AccountManager::m_vAccounts)
        {
            std::string m_sAccount = std::to_string(m_iCount) + ". ";
            m_sAccount += Account.GetDisplayName();
            m_sAccount += " - Last Switch (" + Account.m_sLastSwitchDate + ")";

            m_vReturn.emplace_back(m_sAccount);

            ++m_iCount;
        }

        return m_vReturn;
    }
}


int main()
{
    Console::Init("SSAS"); 
    SetConsoleOutputCP(437); // Required for arrow draw...

    if (!Program::Steam::Init())
        return Program::Error("Couldn't fetch Steam informations.");

    AccountManager::Init();

    while (1)
    {
        static std::string m_sOptions[] =
        {
            "Switch Account", "Add Account", "Edit Account", "New Login", "Exit"
        };
        static constexpr int m_iOptionSize = (sizeof(m_sOptions) / sizeof(m_sOptions[0]));
        static constexpr int m_iExitOption = m_iOptionSize - 1;

        int m_iMainOption = Program::GetSelectionIndex("Select Option:\n", m_sOptions, m_iOptionSize, -1, "\t");

        switch (m_iMainOption)
        {
            // Switch Account
            case 0:
            {
                if (AccountManager::m_vAccounts.empty())
                    Program::Error("You need to first add account.");
                else
                {
                    std::vector<std::string> m_vAccountList = Program::GetFormattedAccountList();
                    int m_iSelectedAccount = Program::GetSelectionIndex("Select Account to switch:\n", m_vAccountList.data(), m_vAccountList.size(), 3, "\t");

                    if (m_iSelectedAccount != -1)
                    {
                        CAccount& Account = AccountManager::m_vAccounts[m_iSelectedAccount];

                        Program::Info();
                        Console::Print(CLR_BYELLOW, '~', "Preparing for account switch...\n\n");
                        if (Program::Steam::IsRunning())
                        {
                            Console::Print(Console::m_uDefaultColor, '~', "Closing steam...\n");
                            Program::Steam::Exit();
                        }

                        std::string m_sAccountVDF = AccountManager::GetPath(Account.m_sName) + "\\" + AccountManager::m_pVDFName;
                        if (CopyFileA(&m_sAccountVDF[0], &Program::Steam::m_sVDFPath[0], FALSE))
                        {
                            bool m_bSomeRegisterFailed = false;
                            if (!Utils::Registry::WriteString(HKEY_CURRENT_USER, Program::Steam::m_pSteamRegistry, "AutoLoginUser", Account.m_sName))
                                m_bSomeRegisterFailed = true;

                            if (!Utils::Registry::WriteDWORD(HKEY_CURRENT_USER, Program::Steam::m_pSteamRegistry, "RememberPassword", 1U))
                                m_bSomeRegisterFailed = true;

                            if (m_bSomeRegisterFailed)
                                Program::Error("Couldn't write login user name.");
                            else
                            {
                                std::vector<std::string> m_SwitchAccountSSFN = Utils::Steam::GetFilesSSFN(AccountManager::GetSSFNPath(Account.m_sName));
                                if (!m_SwitchAccountSSFN.empty())
                                {
                                    std::vector<std::string> m_SteamSSFN = Utils::Steam::GetFilesSSFN(Program::Steam::m_sPath);
                                    for (std::string s : m_SteamSSFN)
                                    {
                                        s = Program::Steam::m_sPath + "\\" + s;
                                        remove(&s[0]);
                                    }

                                    for (std::string s : m_SwitchAccountSSFN)
                                    {
                                        std::string src = AccountManager::GetSSFNPath(Account.m_sName) + "\\" + s;
                                        std::string dst = Program::Steam::m_sPath + "\\" + s;

                                        if (!CopyFileA(&src[0], &dst[0], FALSE))
                                        {
                                            Program::Error("Couldn't write ssfn file.");
                                            break;
                                        }
                                    }
                                }

                                Console::Print(Console::m_uDefaultColor, '~', "Launching steam...\n\n");
                                Program::Steam::Start();

                                // Update last switch date
                                {
                                    char m_cDateTime[32];
                                    struct tm m_tTime;  
                                    time_t m_tNow = time(0);
                                    localtime_s(&m_tTime, &m_tNow);
                                    strftime(m_cDateTime, sizeof(m_cDateTime), "%e %b %Y - %H:%M:%S", &m_tTime);

                                    AccountManager::WriteInfo(Account.m_sName, Account.m_sAlternativeName, m_cDateTime);
                                    AccountManager::Refresh();
                                }

                                Console::Print(CLR_BGREEN, '~', "Successfully switched account...\n");
                                Sleep(5000);

                                // Since switching account removes some required data to allow switching we change this so when user exit steam that she/he can log to new account.
                                Utils::Registry::WriteDWORD(HKEY_CURRENT_USER, Program::Steam::m_pSteamRegistry, "RememberPassword", 0U);
                            }
                        }
                        else
                            Program::Error("Couldn't replace config file.");
                    }
                }
            }
            break;

            // Add Account
            case 1:
            {
                if (Program::Steam::IsRunning())
                {
                    if (Program::Steam::IsRememberPasswordChecked())
                    {
                        std::string m_sLoginName = Program::Steam::GetLoginUser();
                        if (!m_sLoginName.empty())
                        {
                            AccountManager::New(m_sLoginName, Program::Steam::m_sVDFPath);
                            AccountManager::Refresh();

                            #ifdef ENABLE_SSFN

                            std::string m_AccountSSFN = AccountManager::GetSSFNPath(m_sLoginName);
                            CreateDirectoryA(&m_AccountSSFN[0], 0);

                            std::vector<std::string> m_SteamSSFN = Utils::Steam::GetFilesSSFN(Program::Steam::m_sPath);
                            for (std::string s : m_SteamSSFN)
                            {
                                std::string src = Program::Steam::m_sPath + "\\" + s;
                                std::string dst = m_AccountSSFN + "\\" + s;
                                CopyFileA(&src[0], &dst[0], FALSE);
                            }

                            #endif
                        }
                        else
                            Program::Error("Couldn't read logged user name.");
                    }
                    else
                        Program::Error("You need to login to your account with remember password.");
                }
                else
                    Program::Error("Steam needs to be running before adding account.");
            }
            break;

            // Edit Account
            case 2:
            {
                if (AccountManager::m_vAccounts.empty())
                    Program::Error("You need to first add account.");
                else
                {
                    std::vector<std::string> m_vAccountList = Program::GetFormattedAccountList();
                    int m_iSelectedAccount = Program::GetSelectionIndex("Select Account to edit:\n", m_vAccountList.data(), m_vAccountList.size(), 3, "\t");

                    if (m_iSelectedAccount != -1)
                    {
                        CAccount& Account = AccountManager::m_vAccounts[m_iSelectedAccount];

                        Program::Info();
                        Console::Print(Console::m_uDefaultColor, '!', "Alternative Name (left empty to use default): ");

                        std::string m_sAlternativeName;
                        std::getline(std::cin, m_sAlternativeName);

                        AccountManager::WriteInfo(Account.m_sName, m_sAlternativeName);
                        AccountManager::Refresh();
                    }
                }
            }
            break;

            // New Login
            case 3:
            {
                Program::Info();
                if (Program::Steam::IsRunning())
                {
                    Console::Print(Console::m_uDefaultColor, '~', "Closing steam...\n");
                    Program::Steam::Exit();
                }

                Utils::Registry::WriteString(HKEY_CURRENT_USER, Program::Steam::m_pSteamRegistry, "AutoLoginUser", "");
                Utils::Registry::WriteDWORD(HKEY_CURRENT_USER, Program::Steam::m_pSteamRegistry, "RememberPassword", 1U);

                Console::Print(Console::m_uDefaultColor, '~', "Launching steam...\n\n");
                Program::Steam::Start();
            }
            break;

            case m_iExitOption: exit(0);
        }
    }

    return 0;
}