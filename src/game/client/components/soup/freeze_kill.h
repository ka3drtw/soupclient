#ifndef GAME_CLIENT_COMPONENTS_SOUP_FREEZE_KILL_H
#define GAME_CLIENT_COMPONENTS_SOUP_FREEZE_KILL_H
#include <game/client/component.h>
class CFreezeKill : public CComponent
{
public:
    bool m_Sent = false;
    int64_t m_ArmTime = 0;
    int Sizeof() const override { return sizeof(*this); }
    void OnRender() override;
};
#endif
