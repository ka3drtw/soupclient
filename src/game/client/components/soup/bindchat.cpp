#include <engine/shared/config.h>
#include <game/client/gameclient.h>

#include "../chat.h"
#include "../emoticon.h"

#include "bindchat.h"

CBindchat::CBindchat()
{
	OnReset();
}

void CBindchat::ConAddBindchat(IConsole::IResult *pResult, void *pUserData)
{
	const char *aName = pResult->GetString(0);
	const char *aCommand = pResult->GetString(1);

	CBindchat *pThis = static_cast<CBindchat *>(pUserData);
	pThis->AddBind(aName, aCommand);
}

void CBindchat::ConBindchats(IConsole::IResult *pResult, void *pUserData)
{
	CBindchat *pThis = static_cast<CBindchat *>(pUserData);
	char aBuf[BINDCHAT_MAX_NAME + BINDCHAT_MAX_CMD + 32];
	if(pResult->NumArguments() == 1)
	{
		const char *pName = pResult->GetString(0);
		for(const CBind &Bind : pThis->m_vBinds)
		{
			if(str_comp_nocase(Bind.m_aName, pName) == 0)
			{
				str_format(aBuf, sizeof(aBuf), "%s = %s", Bind.m_aName, Bind.m_aCommand);
				pThis->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "bindchat", aBuf);
				return;
			}
		}
		str_format(aBuf, sizeof(aBuf), "%s is not bound", pName);
		pThis->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "bindchat", aBuf);
	}
	else
	{
		for(const CBind &Bind : pThis->m_vBinds)
		{
			str_format(aBuf, sizeof(aBuf), "%s = %s", Bind.m_aName, Bind.m_aCommand);
			pThis->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "bindchat", aBuf);
		}
	}
}

void CBindchat::ConRemoveBindchat(IConsole::IResult *pResult, void *pUserData)
{
	const char *aName = pResult->GetString(0);
	CBindchat *pThis = static_cast<CBindchat *>(pUserData);
	pThis->RemoveBind(aName);
}

void CBindchat::ConRemoveBindchatAll(IConsole::IResult *pResult, void *pUserData)
{
	CBindchat *pThis = static_cast<CBindchat *>(pUserData);
	pThis->RemoveAllBinds();
}

void CBindchat::ConBindchatDefaults(IConsole::IResult *pResult, void *pUserData)
{
	CBindchat *pThis = static_cast<CBindchat *>(pUserData);
	pThis->AddBind("!shrug", "say ¯\\_(ツ)_/¯");
	pThis->AddBind("!flip", "say (╯°□°)╯︵ ┻━┻");
	pThis->AddBind("!unflip", "say ┬─┬ノ( º _ ºノ)");
}

void CBindchat::AddBind(const char *pName, const char *pCommand)
{
	if((pName[0] == '\0' && pCommand[0] == '\0') || m_vBinds.size() >= BINDCHAT_MAX_BINDS)
		return;

	RemoveBind(pName); // Prevent duplicates

	CBind Bind;
	str_copy(Bind.m_aName, pName);
	str_copy(Bind.m_aCommand, pCommand);
	m_vBinds.push_back(Bind);
}

void CBindchat::AddBindDefault(const char *pName, const char *pCommand)
{
	if((pName[0] == '\0' && pCommand[0] == '\0') || m_vBinds.size() >= BINDCHAT_MAX_BINDS)
		return;

	// If a bind for this command is found then don't add this
	for(auto It = m_vBinds.begin(); It != m_vBinds.end(); ++It)
	{
		if(str_comp(It->m_aCommand, pCommand) == 0)
			return;
	}

	CBind Bind;
	Bind.m_Default = true;
	str_copy(Bind.m_aName, pName);
	str_copy(Bind.m_aCommand, pCommand);
	m_vBinds.push_back(Bind);
}

void CBindchat::RemoveBindCommand(const char *pCommand)
{
	if(pCommand[0] == '\0')
		return;
	for(auto It = m_vBinds.begin(); It != m_vBinds.end(); ++It)
	{
		if(str_comp(It->m_aCommand, pCommand) == 0)
		{
			m_vBinds.erase(It);
			return;
		}
	}
}

void CBindchat::RemoveBind(const char *pName)
{
	if(pName[0] == '\0')
		return;
	for(auto It = m_vBinds.begin(); It != m_vBinds.end(); ++It)
	{
		if(str_comp(It->m_aName, pName) == 0)
		{
			m_vBinds.erase(It);
			return;
		}
	}
}

void CBindchat::RemoveBind(int Index)
{
	if(Index >= static_cast<int>(m_vBinds.size()) || Index < 0)
		return;
	auto It = m_vBinds.begin() + Index;
	m_vBinds.erase(It);
}

void CBindchat::RemoveAllBinds()
{
	m_vBinds.clear();
}

int CBindchat::GetBindNoDefault(const char *pCommand)
{
	if(pCommand[0] == '\0')
		return -1;
	for(auto It = m_vBinds.begin(); It != m_vBinds.end(); ++It)
	{
		if(str_comp_nocase(It->m_aCommand, pCommand) == 0 && !It->m_Default)
			return &*It - m_vBinds.data();
	}
	return -1;
}

int CBindchat::GetBind(const char *pCommand)
{
	if(pCommand[0] == '\0')
		return -1;
	for(auto It = m_vBinds.begin(); It != m_vBinds.end(); ++It)
	{
		if(str_comp_nocase(It->m_aCommand, pCommand) == 0)
			return &*It - m_vBinds.data();
	}
	return -1;
}

CBindchat::CBind *CBindchat::Get(int Index)
{
	if(Index < 0 || Index >= (int)m_vBinds.size())
		return nullptr;
	return &m_vBinds[Index];
}

void CBindchat::OnConsoleInit()
{
	IConfigManager *pConfigManager = Kernel()->RequestInterface<IConfigManager>();
	if(pConfigManager)
		pConfigManager->RegisterTCallback(ConfigSaveCallback, this);

	Console()->Register("bindchat", "s[name] r[command]", CFGFLAG_CLIENT, ConAddBindchat, this, "Add a chat bind");
	Console()->Register("bindchats", "?s[name]", CFGFLAG_CLIENT, ConBindchats, this, "Print command executed by this name or all chat binds");
	Console()->Register("unbindchat", "s[name] r[command]", CFGFLAG_CLIENT, ConRemoveBindchat, this, "Remove a chat bind");
	Console()->Register("unbindchatall", "", CFGFLAG_CLIENT, ConRemoveBindchatAll, this, "Removes all chat binds");
	Console()->Register("bindchatdefaults", "", CFGFLAG_CLIENT, ConBindchatDefaults, this, "Adds default chat binds");

	m_pStorage = Kernel()->RequestInterface<IGameStorage>();
	IOHANDLE File = m_pStorage->OpenFile(BINDCHAT_FILE, IOFLAG_READ, IGameStorage::TYPE_ALL);
	if(File)
	{
		io_close(File);
		Console()->ExecuteFile(BINDCHAT_FILE);
	}

	// Default Binds
	AddBindDefault(".war", "war_name_index 1");
	AddBindDefault(".warclan", "war_clan_index 1");
	AddBindDefault(".team", "war_name_index 2");
	AddBindDefault(".teamclan", "war_clan_index 2");
	AddBindDefault(".delwar", "remove_war_name_index 1");
	AddBindDefault(".delwarclan", "remove_war_clan_index 1");
	AddBindDefault(".delteam", "remove_war_name_index 2");
	AddBindDefault(".delteamclan", "remove_war_clan_index 2");

	AddBindDefault(".name", "war_name");
	AddBindDefault(".clan", "war_clan");
	AddBindDefault(".delname", "remove_war_name");
	AddBindDefault(".delclan", "remove_war_clan");

	AddBindDefault("!mute", "add_foe");
	AddBindDefault("!unmute", "remove_foe");
}

void CBindchat::ExecuteBind(int Bind, const char *pArgs)
{
	char aBuf[BINDCHAT_MAX_CMD] = "";
	str_append(aBuf, m_vBinds[Bind].m_aCommand);
	if(pArgs)
	{
		str_append(aBuf, " ");
		str_append(aBuf, pArgs);
	}
	Console()->ExecuteLine(aBuf);
}

bool CBindchat::CheckBindChat(const char *pText)
{
	const char *pSpace = str_find(pText, " ");
	size_t SpaceIndex = pSpace ? pSpace - pText : strlen(pText);
	for(const CBind &Bind : m_vBinds)
	{
		if(str_comp_nocase_num(pText, Bind.m_aName, SpaceIndex) == 0)
			return true;
	}
	return false;
}

bool CBindchat::ChatDoBinds(const char *pText)
{
	if(pText[0] == ' ')
		return false;

	CChat &Chat = GameClient()->m_Chat;
	const char *pSpace = str_find(pText, " ");
	size_t SpaceIndex = pSpace ? pSpace - pText : strlen(pText);
	for(const CBind &Bind : m_vBinds)
	{
		if(str_comp_nocase_num(pText, Bind.m_aName, SpaceIndex) == 0)
		{
			ExecuteBind(&Bind - m_vBinds.data(), pSpace ? pSpace + 1 : nullptr);
			// Add to history (see CChat::SendChatQueued)
			const int Length = str_length(pText);
			CChat::CHistoryEntry *pEntry = Chat.m_History.Allocate(sizeof(CChat::CHistoryEntry) + Length);
			pEntry->m_Team = 0; // All
			str_copy(pEntry->m_aText, pText, Length + 1);
			return true;
		}
	}
	return false;
}

bool CBindchat::ChatDoAutocomplete(bool ShiftPressed)
{
	CChat &Chat = GameClient()->m_Chat;

	if(m_vBinds.size() == 0)
		return false;
	if(*Chat.m_aCompletionBuffer == '\0')
		return false;

	const CBind *pCompletionBind = nullptr;
	int InitialCompletionChosen = Chat.m_CompletionChosen;
	int InitialCompletionUsed = Chat.m_CompletionUsed;

	if(ShiftPressed && Chat.m_CompletionUsed)
		Chat.m_CompletionChosen--;
	else if(!ShiftPressed)
		Chat.m_CompletionChosen++;
	Chat.m_CompletionChosen = (Chat.m_CompletionChosen + m_vBinds.size()) % m_vBinds.size(); // size != 0

	Chat.m_CompletionUsed = true;
	int Index = Chat.m_CompletionChosen;
	for(int i = 0; i < (int)m_vBinds.size(); i++)
	{
		int CommandIndex = (Index + i) % m_vBinds.size();
		if(str_startswith_nocase(m_vBinds.at(CommandIndex).m_aName, Chat.m_aCompletionBuffer))
		{
			pCompletionBind = &m_vBinds.at(CommandIndex);
			Chat.m_CompletionChosen = CommandIndex;
			break;
		}
	}
	//for(const CBind &Bind : m_vBinds)
	//{
	//	if(str_startswith_nocase(Bind.m_aName, Chat.m_aCompletionBuffer))
	//	{
	//		pCompletionBind = &Bind;
	//		Chat.m_CompletionChosen = &Bind - m_vBinds.data();
	//		break;
	//	}
	//}

	// insert the command
	if(pCompletionBind)
	{
		char aBuf[CChat::MAX_LINE_LENGTH];
		// add part before the name
		str_truncate(aBuf, sizeof(aBuf), Chat.m_Input.GetString(), Chat.m_PlaceholderOffset);

		// add the command
		str_append(aBuf, pCompletionBind->m_aName);

		// add separator
		// TODO: figure out if the command would accept an extra param
		// char commandBuf[128];
		// str_next_token(pCompletionBind->m_aCommand, " ", commandBuf, sizeof(commandBuf));
		// CCommandInfo *pInfo = m_pClient->Console()->GetCommandInfo(commandBuf, CFGFLAG_CLIENT, false);
		// if(pInfo && pInfo->m_pParams != '\0')
		const char *pSeperator = " ";
		str_append(aBuf, pSeperator);

		// add part after the name
		str_append(aBuf, Chat.m_Input.GetString() + Chat.m_PlaceholderOffset + Chat.m_PlaceholderLength);

		Chat.m_PlaceholderLength = str_length(pSeperator) + str_length(pCompletionBind->m_aName);
		Chat.m_Input.Set(aBuf);
		Chat.m_Input.SetCursorOffset(Chat.m_PlaceholderOffset + Chat.m_PlaceholderLength);
	}
	else
	{
		Chat.m_CompletionChosen = InitialCompletionChosen;
		Chat.m_CompletionUsed = InitialCompletionUsed;
	}

	return pCompletionBind != nullptr;
}

void CBindchat::WriteLine(const char *pLine)
{
	if(!m_BindchatFile || io_write(m_BindchatFile, pLine, str_length(pLine)) != static_cast<unsigned>(str_length(pLine)) || !io_write_newline(m_BindchatFile))
		return;
}
void CBindchat::ConfigSaveCallback(IConfigManager *pConfigManager, void *pUserData)
{
	CBindchat *pThis = (CBindchat *)pUserData;
	bool Failed = false;
	pThis->m_BindchatFile = pThis->m_pStorage->OpenFile(BINDCHAT_FILE, IOFLAG_WRITE, IGameStorage::TYPE_SAVE);
	if(!pThis->m_BindchatFile)
	{
		dbg_msg("config", "ERROR: opening %s failed", BINDCHAT_FILE);
		return;
	}

	for(CBind &Bind : pThis->m_vBinds)
	{
		// Default binds do not need to be saved because they will get added on launch
		if(Bind.m_Default)
			continue;

		char aBuf[BINDCHAT_MAX_CMD * 2] = "";
		char *pEnd = aBuf + sizeof(aBuf);
		char *pDst;
		str_append(aBuf, "bindchat \"");
		// Escape name
		pDst = aBuf + str_length(aBuf);
		str_escape(&pDst, Bind.m_aName, pEnd);
		str_append(aBuf, "\" \"");
		// Escape command
		pDst = aBuf + str_length(aBuf);
		str_escape(&pDst, Bind.m_aCommand, pEnd);
		str_append(aBuf, "\"");
		pThis->WriteLine(aBuf);
	}

	if(io_sync(pThis->m_BindchatFile) != 0)
		Failed = true;
	if(io_close(pThis->m_BindchatFile) != 0)
		Failed = true;
	pThis->m_BindchatFile = {};
	if(Failed)
		dbg_msg("config", "ERROR: writing to %s failed", BINDCHAT_FILE);
}
