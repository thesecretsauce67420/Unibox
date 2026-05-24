#include "AutoJoin.h"

// beta nameskeeter (trol)
void NameStealOnJoin() {
    std::vector<std::string> names;

    for (int i = 1; i <= I::EngineClient->GetMaxClients(); i++) {
        if (i == I::EngineClient->GetLocalPlayer())
            continue;

        if (!I::EngineClient->IsInGame())
            continue;

        const char* name = F::PlayerUtils.GetPlayerAlias(i);
        if (name && name[0] != '\0') {
            names.push_back(name);
        }
    }

    if (names.empty())
        return;

    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, names.size() - 1);

    std::string chosenName = names[dist(gen)];

    auto pNetChan = reinterpret_cast<CNetChannel*>(I::EngineClient->GetNetChannelInfo());

    NET_SetConVar nameMsg = { "name", chosenName.c_str() };
    pNetChan->SendNetMsg(nameMsg);
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
		}
		else
		{
			I::EngineClient->ClientCmd_Unrestricted("team_ui_setup");
			I::EngineClient->ClientCmd_Unrestricted("menuopen");
			I::EngineClient->ClientCmd_Unrestricted("autoteam");
			I::EngineClient->ClientCmd_Unrestricted("menuclosed");
			NameStealOnJoin();
		}
	}
}
