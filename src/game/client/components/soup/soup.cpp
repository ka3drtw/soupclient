#include <engine/graphics.h>
#include <engine/shared/config.h>
#include <game/generated/protocol.h>
#include <engine/shared/json.h>
#include <game/version.h>

#include "../chat.h"
#include "../emoticon.h"

#include <game/client/animstate.h>
#include <game/client/render.h>
#include <game/client/ui.h>


#include <base/system.h>
#include <game/client/components/binds.h>
#include <engine/keys.h>
#include <engine/shared/config.h>
#include "soup.h"
#include <game/client/gameclient.h>


static constexpr const char *SOUPCLIENT_INFO_URL = "https://github.com/ka3drtw/soupclient/releases";

CSoup::CSoup()
{
	OnReset();
}

void CSoup::ConRandomTee(IConsole::IResult *pResult, void *pUserData) {}

void CSoup::ConchainRandomColor(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
	CSoup *pThis = static_cast<CSoup *>(pUserData);
	// resolve type to randomize
	// check length of type (0 = all, 1 = body, 2 = feet, 3 = skin, 4 = flag)
	bool RandomizeBody = false;
	bool RandomizeFeet = false;
	bool RandomizeSkin = false;
	bool RandomizeFlag = false;

	if(pResult->NumArguments() == 0)
	{
		RandomizeBody = true;
		RandomizeFeet = true;
		RandomizeSkin = true;
		RandomizeFlag = true;
	}
	else if(pResult->NumArguments() == 1)
	{
		const char *Type = pResult->GetString(0);
		int Length = Type ? str_length(Type) : 0;
		if(Length == 1 && Type[0] == '0')
		{ // Randomize all
			RandomizeBody = true;
			RandomizeFeet = true;
			RandomizeSkin = true;
			RandomizeFlag = true;
		}
		else if(Length == 1)
		{
			// randomize body
			RandomizeBody = Type[0] == '1';
		}
		else if(Length == 2)
		{
			// check for body and feet
			RandomizeBody = Type[0] == '1';
			RandomizeFeet = Type[1] == '1';
		}
		else if(Length == 3)
		{
			// check for body, feet and skin
			RandomizeBody = Type[0] == '1';
			RandomizeFeet = Type[1] == '1';
			RandomizeSkin = Type[2] == '1';
		}
		else if(Length == 4)
		{
			// check for body, feet, skin and flag
			RandomizeBody = Type[0] == '1';
			RandomizeFeet = Type[1] == '1';
			RandomizeSkin = Type[2] == '1';
			RandomizeFlag = Type[3] == '1';
		}
	}

	if(RandomizeBody)
		RandomBodyColor();
	if(RandomizeFeet)
		RandomFeetColor();
	if(RandomizeSkin)
		RandomSkin(pUserData);
	if(RandomizeFlag)
		RandomFlag(pUserData);
	pThis->m_pClient->SendInfo(false);
}

bool CSoup::NeedUpdate()
{
	return str_comp(m_aVersionStr, "0") != 0;
}

void CSoup::ResetSoupClientInfoTask()
{
	if(m_pSoupClientInfoTask)
	{
		m_pSoupClientInfoTask->Abort();
		m_pSoupClientInfoTask = NULL;
	}
}

void CSoup::OnRender()
{
	
	GoresMode();

	if(m_pSoupClientInfoTask)
	{
		if(m_pSoupClientInfoTask->State() == EHttpState::DONE)
		{
			FinishSoupClientInfo();
			ResetSoupClientInfoTask();
		}
	}
}

typedef std::tuple<int, int, int> TVersion;
static const TVersion gs_InvalidTCVersion = std::make_tuple(-1, -1, -1);

TVersion ToTCVersion(char *pStr)
{
	int aVersion[3] = {0, 0, 0};
	const char *p = strtok(pStr, ".");

	for(int i = 0; i < 3 && p; ++i)
	{
		if(!str_isallnum(p))
			return gs_InvalidTCVersion;

		aVersion[i] = str_toint(p);
		p = strtok(NULL, ".");
	}

	if(p)
		return gs_InvalidTCVersion;

	return std::make_tuple(aVersion[0], aVersion[1], aVersion[2]);
}

void CSoup::FinishSoupClientInfo()
{
	json_value *pJson = m_pSoupClientInfoTask->ResultJson();
	if(!pJson)
		return;
	const json_value &Json = *pJson;
	const json_value &CurrentVersion = Json["version"];

	if(CurrentVersion.type == json_string)
	{
		char aNewVersionStr[64];
		str_copy(aNewVersionStr, CurrentVersion);
		char aCurVersionStr[64];
		str_copy(aCurVersionStr, SOUPCLIENT_VERSION);
		if(ToTCVersion(aNewVersionStr) > ToTCVersion(aCurVersionStr))
		{
			str_copy(m_aVersionStr, CurrentVersion);
		}
		else
		{
			m_aVersionStr[0] = '0';
			m_aVersionStr[1] = '\0';
		}
	}

	json_value_free(pJson);
}

void CSoup::FetchSoupClientInfo()
{
	if(m_pSoupClientInfoTask && !m_pSoupClientInfoTask->Done())
		return;
	char aUrl[256];
	str_copy(aUrl, SOUPCLIENT_INFO_URL);
	m_pSoupClientInfoTask = HttpGet(aUrl);
	m_pSoupClientInfoTask->Timeout(CTimeout{10000, 0, 500, 10});
	m_pSoupClientInfoTask->IpResolve(IPRESOLVE::V4);
	Http()->Run(m_pSoupClientInfoTask);
}

//soup
void CSoup::GoresMode()
{
    if(!g_Config.m_ClGoresMode && m_GoresModeRebound)
    {
        int Key = Input()->FindKeyByName(g_Config.m_ClGoresModeKey);
        if(Key != KEY_UNKNOWN)
        {
            GameClient()->m_Binds.Bind(Key, m_ClGoresModeSaved);
        }
        m_GoresModeRebound = false;
    }

    if(g_Config.m_ClGoresMode && !m_GoresModeRebound)
    {
        int Key = Input()->FindKeyByName(g_Config.m_ClGoresModeKey);
        if(Key != KEY_UNKNOWN)
        {
            if(!m_GoresModeSaved)
            {
                const char *pBind = GameClient()->m_Binds.Get(Key, 0);
                if(pBind)
                {
                    str_copy(m_ClGoresModeSaved, pBind, sizeof(m_ClGoresModeSaved));
                }
                m_GoresModeSaved = true;
            }
            
            GameClient()->m_Binds.Bind(Key, "+fire;+prevweapon");
            m_GoresModeRebound = true;
        }
    }

    CCharacterCore Core = GameClient()->m_PredictedPrevChar;
    if(g_Config.m_ClGoresModeDisableIfWeapons && g_Config.m_ClGoresMode)
    {
        bool HasWeapons = (Core.m_aWeapons[WEAPON_GRENADE].m_Got || 
                         Core.m_aWeapons[WEAPON_LASER].m_Got || 
                         Core.m_ExplosionGun || 
                         Core.m_aWeapons[WEAPON_SHOTGUN].m_Got);

        if(HasWeapons)
        {
            g_Config.m_ClGoresMode = 0;
            m_WeaponsGot = true;
        }
        else if(m_WeaponsGot)
        {
            g_Config.m_ClGoresMode = 1;
            m_WeaponsGot = false;
        }
    }

    if(GameClient()->m_Snap.m_pLocalCharacter && g_Config.m_ClGoresMode)
    {
        if(GameClient()->m_Snap.m_pLocalCharacter->m_Weapon == 0)
        {
            GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_WantedWeapon = 2;
        }
    }
}

void CSoup::OnInit()
{
	TextRender()->SetCustomFace(g_Config.m_ClCustomFont);
	TextRender()->SetCustomFace(g_Config.m_ClCustomFont);
	m_pGraphics = Kernel()->RequestInterface<IEngineGraphics>();
	FetchSoupClientInfo();

	// soup
	int Key = Input()->FindKeyByName(g_Config.m_ClGoresModeKey);
	if(Key == KEY_UNKNOWN)
		dbg_msg("[Soup]", "Invalid key: %s", g_Config.m_ClGoresModeKey);
	else
	{
		const CBinds::CBindSlot BindSlot = GameClient()->m_Binds.GetBindSlot(g_Config.m_ClGoresModeKey);
		*g_Config.m_ClGoresModeSaved = *GameClient()->m_Binds.m_aapKeyBindings[BindSlot.m_ModifierMask][BindSlot.m_Key];

		// Tells you what the bind is
		char aBuf[1024];
		str_format(aBuf, sizeof(aBuf), "Gores Mode saved bind currently is: %s", g_Config.m_ClGoresModeSaved);
		dbg_msg("[Soup]", aBuf);

		// Binds the mouse to the saved bind, also doe
		GameClient()->m_Binds.Bind(Key, g_Config.m_ClGoresModeSaved);
	}

}

void CSoup::OnConsoleInit()
{
	Console()->Register("reply_to_ping", "r[message]", CFGFLAG_CLIENT, ConReplyToPing, this, "Reply to last ping");
    
    m_aLastPingName[0] = '\0';
    m_aLastPingMessage[0] = '\0';
    m_LastPingTeam = 0;
	Console()->Register("sc_random_player", "s[type]", CFGFLAG_CLIENT, ConRandomTee, this, "Randomize player color (0 = all, 1 = body, 2 = feet, 3 = skin, 4 = flag) example: 0011 = randomize skin and flag [number is position] ");
	Console()->Chain("sc_random_player", ConchainRandomColor, this);

	Console()->Chain(
		"sc_allow_any_resolution", [](IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData) {
			pfnCallback(pResult, pCallbackUserData);
			((CSoup *)pUserData)->SetForcedAspect();
		},
		this);
}

void CSoup::ConReplyToPing(IConsole::IResult *pResult, void *pUserData)
{
    CSoup *pSelf = (CSoup *)pUserData;
    
    if(pSelf->m_aLastPingName[0] == '\0')
    {
        pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "reply_to_ping", "No recent ping found");
        return;
    }

    const char *pMessage = pResult->NumArguments() > 0 ? pResult->GetString(0) : "";
    
    char aResponse[256];
    str_format(aResponse, sizeof(aResponse), "%s: %s", 
              pSelf->m_aLastPingName,
              pMessage);
    
    pSelf->GameClient()->m_Chat.SendChat(pSelf->m_LastPingTeam, aResponse);
}




//soup
void CSoup::SetForcedAspect()
{
	// TODO: Fix flashing on windows
	int State = Client()->State();
	bool Force = true;
	if(g_Config.m_ClAllowAnyRes == 0)
		;
	else if(State == CClient::EClientState::STATE_DEMOPLAYBACK)
		Force = false;
	else if(State == CClient::EClientState::STATE_ONLINE && GameClient()->m_GameInfo.m_AllowZoom && !GameClient()->m_Menus.IsActive())
		Force = false;
	Graphics()->SetForcedAspect(Force);
}

void CSoup::OnStateChange(int OldState, int NewState)
{
	SetForcedAspect();
}

void CSoup::OnNewSnapshot()
{
	SetForcedAspect();
}
//soup

void CSoup::RandomBodyColor()
{
	g_Config.m_ClPlayerColorBody = ColorHSLA((std::rand() % 100) / 100.0f, (std::rand() % 100) / 100.0f, (std::rand() % 100) / 100.0f, 1.0f).Pack(false);
}

void CSoup::RandomFeetColor()
{
	g_Config.m_ClPlayerColorFeet = ColorHSLA((std::rand() % 100) / 100.0f, (std::rand() % 100) / 100.0f, (std::rand() % 100) / 100.0f, 1.0f).Pack(false);
}

void CSoup::RandomSkin(void *pUserData)
{
	CSoup *pThis = static_cast<CSoup *>(pUserData);
	// get the skin count
	int SkinCount = (int)pThis->m_pClient->m_Skins.GetSkinsUnsafe().size();

	// get a random skin number
	int SkinNumber = std::rand() % SkinCount;

	// get all skins as a maps
	const std::unordered_map<std::string_view, std::unique_ptr<CSkin>> &Skins = pThis->m_pClient->m_Skins.GetSkinsUnsafe();

	// map to array
	int Counter = 0;
	std::vector<std::pair<std::string_view, CSkin *>> SkinArray;
	for(const auto &Skin : Skins)
	{
		if(Counter == SkinNumber)
		{
			// set the skin name
			const char *SkinName = Skin.first.data();
			str_copy(g_Config.m_ClPlayerSkin, SkinName, sizeof(g_Config.m_ClPlayerSkin));
		}
		Counter++;
	}
}

bool CSoup::SendNonDuplicateMessage(int Team, const char *pLine)
{
	if(str_comp(pLine, m_PreviousOwnMessage) != 0)
	{
		GameClient()->m_Chat.SendChat(Team, pLine);
		return true;
	}
	str_copy(m_PreviousOwnMessage, pLine);
	return false;
}

bool LineShouldHighlight(const char *pLine, const char *pName)
{
	const char *pHL = str_utf8_find_nocase(pLine, pName);
	if(pHL)
	{
		int Length = str_length(pName);
		if(Length > 0 && (pLine == pHL || pHL[-1] == ' ') && (pHL[Length] == 0 || pHL[Length] == ' ' || pHL[Length] == '.' || pHL[Length] == '!' || pHL[Length] == ',' || pHL[Length] == '?' || pHL[Length] == ':'))
			return true;
	}
	return false;
}

void CSoup::RandomFlag(void *pUserData)
{
	CSoup *pThis = static_cast<CSoup *>(pUserData);
	// get the flag count
	int FlagCount = pThis->m_pClient->m_CountryFlags.Num();

	// get a random flag number
	int FlagNumber = std::rand() % FlagCount;

	// get the flag name
	const CCountryFlags::CCountryFlag *pFlag = pThis->m_pClient->m_CountryFlags.GetByIndex(FlagNumber);

	// set the flag code as number
	g_Config.m_PlayerCountry = pFlag->m_CountryCode;
}

void CSoup::OnMessage(int MsgType, void *pRawMsg)
{
	if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
		return;
	if(MsgType == NETMSGTYPE_SV_CHAT)
	{
		
		CNetMsg_Sv_Chat *pMsg = (CNetMsg_Sv_Chat *)pRawMsg;
		int ClientId = pMsg->m_ClientId;

		if(ClientId < 0 || ClientId > MAX_CLIENTS)
			return;
		int LocalId = GameClient()->m_Snap.m_LocalClientId;
		if(ClientId == LocalId)
			str_copy(m_PreviousOwnMessage, pMsg->m_pMessage);

		bool PingMessage = false;

		bool ValidIds = !(GameClient()->m_aLocalIds[0] < 0 || (GameClient()->Client()->DummyConnected() && GameClient()->m_aLocalIds[1] < 0));

		if(ValidIds && ClientId >= 0 && ClientId != GameClient()->m_aLocalIds[0] && (!GameClient()->Client()->DummyConnected() || ClientId != GameClient()->m_aLocalIds[1]))
		{
			PingMessage |= LineShouldHighlight(pMsg->m_pMessage, GameClient()->m_aClients[GameClient()->m_aLocalIds[0]].m_aName);
			PingMessage |= GameClient()->Client()->DummyConnected() && LineShouldHighlight(pMsg->m_pMessage, GameClient()->m_aClients[GameClient()->m_aLocalIds[1]].m_aName);
		}

		if(pMsg->m_Team == TEAM_WHISPER_RECV)
			PingMessage = true;

		if(!PingMessage)
			return;

		char aPlayerName[MAX_NAME_LENGTH];
		str_copy(aPlayerName, GameClient()->m_aClients[ClientId].m_aName, sizeof(aPlayerName));

		bool PlayerMuted = GameClient()->m_aClients[ClientId].m_Foe || GameClient()->m_aClients[ClientId].m_ChatIgnore;
		if(g_Config.m_ClAutoReplyMuted && PlayerMuted)
		{
			char aBuf[256];
			if(pMsg->m_Team == TEAM_WHISPER_RECV)
			{
				str_format(aBuf, sizeof(aBuf), "/w %s %s", aPlayerName, g_Config.m_ClAutoReplyMutedMessage);
				SendNonDuplicateMessage(0, aBuf);
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), "%s: %s", aPlayerName, g_Config.m_ClAutoReplyMutedMessage);
				SendNonDuplicateMessage(0, aBuf);
			}
			return;
		}

		bool WindowActive = m_pGraphics && m_pGraphics->WindowActive();
		if(g_Config.m_ClAutoReplyMinimized && !WindowActive && m_pGraphics)
		{
			char aBuf[256];
			if(pMsg->m_Team == TEAM_WHISPER_RECV)
			{
				str_format(aBuf, sizeof(aBuf), "/w %s %s", aPlayerName, g_Config.m_ClAutoReplyMinimizedMessage);
				SendNonDuplicateMessage(0, aBuf);
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), "%s: %s", aPlayerName, g_Config.m_ClAutoReplyMinimizedMessage);
				SendNonDuplicateMessage(0, aBuf);
			}
			return;
		}
        const char *pName = GameClient()->m_aClients[GameClient()->m_aLocalIds[0]].m_aName;
        if(str_find_nocase(pMsg->m_pMessage, pName))
        {
            str_copy(m_aLastPingName, GameClient()->m_aClients[pMsg->m_ClientId].m_aName, sizeof(m_aLastPingName));
            str_copy(m_aLastPingMessage, pMsg->m_pMessage, sizeof(m_aLastPingMessage));
            m_LastPingTeam = pMsg->m_Team;
        }
    }

	if(MsgType == NETMSGTYPE_SV_VOTESET)
	{
		CNetMsg_Sv_VoteSet *pMsg = (CNetMsg_Sv_VoteSet *)pRawMsg;
		if(pMsg->m_Timeout)
		{
			char aDescription[VOTE_DESC_LENGTH];
			char aReason[VOTE_REASON_LENGTH];
			str_copy(aDescription, pMsg->m_pDescription);
			str_copy(aReason, pMsg->m_pReason);
			bool KickVote = str_startswith(aDescription, "Kick ") != 0 ? true : false;
			bool SpecVote = str_startswith(aDescription, "Pause ") != 0 ? true : false;
			bool SettingVote = !KickVote && !SpecVote;
			bool RandomMapVote = SettingVote && str_find_nocase(aDescription, "random");
			bool MapCoolDown = SettingVote && (str_find_nocase(aDescription, "change map") || str_find_nocase(aDescription, "no not change map"));
			bool CategoryVote = SettingVote && (str_find_nocase(aDescription, "☐") || str_find_nocase(aDescription, "☒"));
			bool FunVote = SettingVote && str_find_nocase(aDescription, "funvote");
			bool MapVote = SettingVote && !RandomMapVote && !MapCoolDown && !CategoryVote && !FunVote && (str_find_nocase(aDescription, "Map:") || str_find_nocase(aDescription, "★") || str_find_nocase(aDescription, "✰"));

			if(g_Config.m_ClAutoVoteWhenFar && (MapVote || RandomMapVote))
			{
				int RaceTime = 0;
				if(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags & GAMESTATEFLAG_RACETIME)
					RaceTime = (Client()->GameTick(g_Config.m_ClDummy) + m_pClient->m_Snap.m_pGameInfoObj->m_WarmupTimer) / Client()->GameTickSpeed();

				if(RaceTime / 60 >= g_Config.m_ClAutoVoteWhenFarTime)
				{
					CGameClient::CClientData *pVoteCaller = nullptr;
					for(int i = 0; i < MAX_CLIENTS; i++)
					{
						if(!m_pClient->m_aStats[i].IsActive())
							continue;

						char aBuf[MAX_NAME_LENGTH + 4];
						str_format(aBuf, sizeof(aBuf), "\'%s\'", m_pClient->m_aClients[i].m_aName);
						if(str_find_nocase(aBuf, pMsg->m_pDescription) == 0)
						{
							pVoteCaller = &m_pClient->m_aClients[i];
						}
					}
					if(pVoteCaller)
					{
						bool Friend = pVoteCaller->m_Friend;
						bool SameTeam = m_pClient->m_Teams.Team(m_pClient->m_Snap.m_LocalClientId) == pVoteCaller->m_Team && pVoteCaller->m_Team != 0;

						if(!Friend && !SameTeam)
						{
							GameClient()->m_Voting.Vote(-1);
							if(str_comp(g_Config.m_ClAutoVoteWhenFarMessage, "") != 0)
								SendNonDuplicateMessage(0, g_Config.m_ClAutoVoteWhenFarMessage);
						}
					}
				}
			}
		}
	}
}