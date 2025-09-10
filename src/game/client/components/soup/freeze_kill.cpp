#include <base/system.h>
#include <engine/shared/config.h>
#include <game/client/components/chat.h>
#include <game/client/gameclient.h>
#include <game/gamecore.h>
#include "freeze_kill.h"
void CFreezeKill::OnRender()
{
    if(!g_Config.m_ClKillOnFreeze)
    {
        m_Sent = false;
        m_ArmTime = 0;
        return;
    }
    const int Local = GameClient()->m_Snap.m_LocalClientId;
    if(Local < 0 || Local >= MAX_CLIENTS)
        return;
    const auto &LocalClient = GameClient()->m_aClients[Local];
    const CCharacterCore &Core = LocalClient.m_Predicted;
    const bool InTileFreeze = Core.m_IsInFreeze;
    const bool DeepOrLive = LocalClient.m_FreezeEnd > 0 || LocalClient.m_DeepFrozen || LocalClient.m_LiveFrozen;
    const bool IsFrozen = (InTileFreeze || DeepOrLive);
    if(!IsFrozen || LocalClient.m_Paused || LocalClient.m_Spec)
    {
        m_Sent = false;
        m_ArmTime = 0;
        return;
    }
    if(m_Sent)
        return;
    const int64_t Now = time_get();
    const int64_t Wait = (int64_t)g_Config.m_ClKillOnFreezeWaitMs * (time_freq() / 1000);
    if(Wait > 0)
    {
        if(m_ArmTime == 0)
            m_ArmTime = Now + Wait;
        if(Now < m_ArmTime)
            return;
    }
    Console()->ExecuteLine("kill");
    m_Sent = true;
}
