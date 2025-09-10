#ifndef GAME_CLIENT_COMPONENTS_SOUP_BG_DRAW_H
#define GAME_CLIENT_COMPONENTS_SOUP_BG_DRAW_H
#include <game/client/component.h>
#include <engine/client/enums.h>
#include <engine/console.h>
#include <array>
#include <list>
#include <optional>
#define BG_DRAW_MAX_POINTS_PER_ITEM 1024
class CBgDrawItem;
class CBgDraw : public CComponent
{
private:
    std::array<std::optional<CBgDrawItem *>, NUM_DUMMIES> m_apActiveItems;
    std::array<std::optional<vec2>, NUM_DUMMIES> m_aLastPos;
    std::list<CBgDrawItem> *m_pvItems;
    static void ConBgDraw(IConsole::IResult *pResult, void *pUserData);
    static void ConBgDrawErase(IConsole::IResult *pResult, void *pUserData);
    static void ConBgDrawReset(IConsole::IResult *pResult, void *pUserData);
    static void ConBgDrawSave(IConsole::IResult *pResult, void *pUserData);
    static void ConBgDrawLoad(IConsole::IResult *pResult, void *pUserData);
    void Reset();
    bool Save(const char *pFile);
    bool Load(const char *pFile);
    template<typename... Args>
    CBgDrawItem *AddItem(Args &&... args);
    void MakeSpaceFor(size_t Count);
public:
    enum class InputMode
    {
        NONE = 0,
        DRAW = 1,
        ERASE = 2,
    };
    std::array<InputMode, NUM_DUMMIES> m_aInputData{};
    int Sizeof() const override { return sizeof(*this); }
    void OnConsoleInit() override;
    void OnRender() override;
    void OnMapLoad() override;
    void OnShutdown() override;
    CBgDraw();
    ~CBgDraw() override;
};
#endif
