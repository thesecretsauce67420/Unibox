#include "AutoJoin.h"
#include "../Misc/Misc.h"
#include "../Configs/Configs.h"

// helpers
// pasted from CMisc
bool LoadLines(const char* szFileName, std::vector<std::string>& vLines, const char* szDefaultContent)
{
	vLines.clear();

	std::string sPath = F::Configs.m_sConfigPath + szFileName;

	if (!std::filesystem::exists(sPath) && szDefaultContent)
	{
		std::ofstream newFile(sPath);
		if (newFile.good())
			newFile << szDefaultContent;
	}

	std::ifstream file(sPath);
	if (!file.good())
		return false;

	std::string line;
	while (std::getline(file, line))
	{
		if (line.empty() || line.find("//") == 0)
			continue;

		vLines.push_back(line);
	}

	return !vLines.empty();
}

std::string GetRandomLine(const std::vector<std::string>& lines)
{
    if (lines.empty())
        return "";

    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::uniform_int_distribution<> dist(0, lines.size() - 1);
    return lines[dist(gen)];
}

// beta namechanger
void ChangeNameOnJoin() {
	std::vector<std::string> lines;
	if (LoadLines("namechange.txt", lines, "THE SPECTRE\nSPECTRE SPECTRE SPECTRE\ndefault unibox namechange"))
	{
    	std::string randomLine = GetRandomLine(lines);
		auto pNetChan = reinterpret_cast<CNetChannel*>(I::EngineClient->GetNetChannelInfo());
		if (!pNetChan) return;
    	NET_SetConVar nameMsg = { "name", randomLine.c_str() };
   		pNetChan->SendNetMsg(nameMsg);
	}
}

// Doesnt work with custom huds!1!!
void CAutoJoin::Run(CTFPlayer* pLocal)
{
	static Timer tJoinTimer{}, tRandomTimer{};
	static int iRandomClass = 0;

	int iDesiredClass = Vars::Misc::Automation::ForceClass.Value;

	if (Vars::Misc::Automation::RandomClass.Value)
	{
		static float flRandomInterval = 0.f;
		if (!iRandomClass || tRandomTimer.Run(flRandomInterval))
		{
			int iExclude = Vars::Misc::Automation::RandomClassExclude.Value;
			do { iRandomClass = SDK::RandomInt(1, 9); }
			while (iExclude & (1 << (iRandomClass - 1)));
			flRandomInterval = SDK::RandomFloat(Vars::Misc::Automation::RandomClassInterval.Value.Min, Vars::Misc::Automation::RandomClassInterval.Value.Max) * 60.f;
		}
		iDesiredClass = iRandomClass;
	}
	else
		iRandomClass = 0;

	if (iDesiredClass && tJoinTimer.Run(1.f))
	{
		if (pLocal->IsInValidTeam())
		{
			if (pLocal->IsAlive() && pLocal->m_iClass() == iDesiredClass)
				return;

			I::EngineClient->ClientCmd_Unrestricted(std::format("joinclass {}", m_aClassNames[iDesiredClass - 1]).c_str());
			I::EngineClient->ClientCmd_Unrestricted("menuclosed");
			if (Vars::Misc::Automation::ChangeNameOnJoin) ChangeNameOnJoin();
		}
		else
		{
			I::EngineClient->ClientCmd_Unrestricted("team_ui_setup");
			I::EngineClient->ClientCmd_Unrestricted("menuopen");
			I::EngineClient->ClientCmd_Unrestricted("autoteam");
			I::EngineClient->ClientCmd_Unrestricted("menuclosed");
		}
	}
}
