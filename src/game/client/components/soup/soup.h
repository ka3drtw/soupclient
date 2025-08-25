#ifndef GAME_CLIENT_COMPONENTS_SOUP_H
#define GAME_CLIENT_COMPONENTS_SOUP_H
#include <game/client/component.h>
#include <engine/shared/http.h>

class CSoup : public CComponent
{
	static void ConRandomTee(IConsole::IResult *pResult, void *pUserData);
	static void ConchainRandomColor(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);
	static void RandomBodyColor();
	static void RandomFeetColor();
	static void RandomSkin(void *pUserData);
	static void RandomFlag(void *pUserData);
	char m_PreviousOwnMessage[2048] = {};
    class IEngineGraphics *m_pGraphics = nullptr;
	bool SendNonDuplicateMessage(int Team, const char *pLine);
	
	bool m_WeaponsGot;
    static void ConReplyToPing(IConsole::IResult *pResult, void *pUserData);
    void DoReplyToPing();
    char m_aLastPingName[32];
    char m_aLastPingMessage[256];
    int m_LastPingTeam;
	
	bool m_GoresModeWasOn;
    bool m_KogModeRebound;
	bool m_GoresServer;
    bool m_GoresModeRebound;
    bool m_GoresModeSaved;
    char m_ClGoresModeSaved[128];

public:
	CSoup();
	virtual int Sizeof() const override { return sizeof(*this); }
	void OnInit() override;
	void OnMessage(int MsgType, void *pRawMsg) override;
	void FetchSoupClientInfo();
	void OnRender() override;
	std::shared_ptr<CHttpRequest> m_pSoupClientInfoTask = nullptr;
	void LoadSoupClientInfoJson();
	void FinishSoupClientInfo();
	void ResetSoupClientInfoTask();
	void SetForcedAspect();
	bool NeedUpdate();
	void OnStateChange(int OldState, int NewState) override;
	void OnNewSnapshot() override;

	//soup
	void GoresMode();

	virtual void OnConsoleInit() override;
	char m_aVersionStr[10] = "0";
};

#endif
