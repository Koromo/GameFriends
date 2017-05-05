#ifndef GAMEFRIENDS_APPLICATION_H
#define GAMEFRIENDS_APPLICATION_H

#include "physicsdebug.h"
#include "../physics/physicsworld.h"
#include "../scene/scene.h"
#include "foundation/prerequest.h"
#include <string>
#include <memory>
#include <chrono>
#include <deque>

GF_NAMESPACE_BEGIN

class Window;
class Level;

struct AppSetup
{
    size_t clientWidth;
    size_t clientHeight;
    std::string title;
    size_t frameRate;
};

class Application
{
private:
    AppSetup setup_;
    decltype(std::chrono::microseconds().count()) frameTime_us_;
    std::deque<float> latestDeltaTimes_s_;

public:
    virtual ~Application() = default;

    std::shared_ptr<Window> window;
    PhysicsWorld physicsWorld;
    PhysicsDebug physicsDebug;
    RenderWorld renderWorld;
    RenderCamera renderCamera;

    void go();
    void quit();

    void setupApp(const AppSetup& setup);
    void setFrameRate(size_t fps);
    size_t latestFrameRate(size_t collectFrames = 60);

    virtual void startup() {}
    virtual void shutdown() {}
    virtual void beginPlay() {}
    virtual void endPlay() {}
    virtual void tick(float dt_s) {}
};

GF_NAMESPACE_END

#endif