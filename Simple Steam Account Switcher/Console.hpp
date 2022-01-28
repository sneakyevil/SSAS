#pragma once

enum m_eConsoleColors : unsigned short
{
	CLR_BGREEN = 0xA,
	CLR_BRED = 0xC,
	CLR_BYELLOW = 0xE,
	CLR_BWHITE = 0xF,
	CLR_WBLACK = 0xF0,
};

namespace Console
{
	HANDLE m_hHandle = nullptr;
	CONSOLE_SCREEN_BUFFER_INFO m_cScreen;
	unsigned int m_uDefaultColor;

	void Init(const char* m_pTitle)
	{
		m_hHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		GetConsoleScreenBufferInfo(m_hHandle, &m_cScreen);
		m_uDefaultColor = m_cScreen.wAttributes;

		SetConsoleTitleA(m_pTitle);
	}

	void Clear()
	{
		GetConsoleScreenBufferInfo(m_hHandle, &m_cScreen);

		int m_iCount = m_cScreen.dwSize.X * m_cScreen.dwSize.Y;
		COORD m_cCoords = { 0, 0 };
		DWORD m_dCount = 0x0;

		FillConsoleOutputCharacterA(m_hHandle, ' ', m_iCount, m_cCoords, &m_dCount);
		FillConsoleOutputAttribute(m_hHandle, m_cScreen.wAttributes, m_iCount, m_cCoords, &m_dCount);
		SetConsoleCursorPosition(m_hHandle, m_cCoords);
	}

	void SetColor(unsigned short m_uColor)
	{
		SetConsoleTextAttribute(m_hHandle, static_cast<WORD>(m_uColor));
	}

	void Print(unsigned int m_uColor, char m_cBracket, std::string m_sText)
	{
		SetColor(CLR_BWHITE);
		std::cout << "[ " << m_cBracket << " ] ";

		SetColor(m_uColor);
		std::cout << m_sText;

		SetColor(m_uDefaultColor);
	}

	void Print(unsigned int m_uColor, std::string m_sText)
	{
		SetColor(m_uColor);
		std::cout << m_sText;

		SetColor(m_uDefaultColor);
	}

	void DrawSelection(int m_iIndex, std::string* m_pList, int m_iSize, int m_iDeltaShow, std::string m_sPrefix)
	{
		int m_iStart = m_iIndex - m_iDeltaShow;
		int m_iEnd = m_iIndex + m_iDeltaShow;

		if (0 > m_iStart)
		{
			m_iEnd += -m_iStart;
			m_iStart = 0;
		}
		else if (m_iEnd >= m_iSize)
		{
			m_iStart -= m_iEnd - m_iSize + 1;
			m_iEnd = m_iSize - 1;
		}
		m_iStart = max(0, min(m_iStart, m_iSize - 1));
		m_iEnd = max(0, min(m_iEnd, m_iSize - 1));

		static const char* m_pScrollInfo[3] = { "\x16\n", "\x1E\n", "\x1F\n" };

		Print(CLR_BWHITE, m_sPrefix + m_pScrollInfo[((m_iStart - 1) >= 0) ? 1 : 0]);
		for (int i = m_iStart; m_iEnd >= i; ++i)
		{
			Print(CLR_BWHITE, m_sPrefix + " ");
			Print(i == m_iIndex ? CLR_WBLACK : CLR_BWHITE, m_pList[i] + "\n");
		}
		Print(CLR_BWHITE, m_sPrefix + m_pScrollInfo[((m_iEnd + 1) < m_iSize) ? 2 : 0]);
	}
}