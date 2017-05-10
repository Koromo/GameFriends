#include "application.h"
#include "input.h"
#include "../audio/audiomanager.h"
#include "../scene/scene.h"
#include "../scene/debugdraw.h"
#include "../render/rendersystem.h"
#include "../engine/logging.h"
#include "../engine/resource.h"
#include "../engine/filesystem.h"
#include "../windowing/window.h"
#include "../windowing/windowsinc.h"
#include "foundation/string.h"
#include "foundation/matrix44.h"
#include "foundation/math.h"

GF_NAMESPACE_BEGIN

namespace
{
    LRESULT CALLBACK windowProc(HWND window, UINT msg, WPARAM wp, LPARAM lp)
    {
        inputDevice.onWinMessage(msg, wp, lp);
        switch (msg)
        {
        case WM_CLOSE:
            PostQuitMessage(0);
            break;

        default:
            break;
        }

        return DefWindowProc(window, msg, wp, lp);
    }

    const size_t MAX_FRAME_RATE = 200;
}

void Application::go()
{
    /* Initialize */

    // Logging 
    logManager.startup("enginelog.txt");

    setup_.clientWidth = 640;
    setup_.clientHeight = 480;
    setup_.title = "GameFriends";
    setup_.frameRate = 60;

    startup();

    const auto inst = GetModuleHandle(NULL);

    WNDCLASSEX wc;
    wc.cbSize = sizeof(wc);
    wc.style = 0;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = inst;
    wc.lpfnWndProc = windowProc;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    wc.lpszMenuName = NULL;
    wc.lpszClassName = widen(setup_.title).c_str();

    if (FAILED(RegisterClassEx(&wc)))
    {
        GF_LOG_ERROR("Game window creation error. Failed to register class.");
        throw WindowsException("Window class register failed.");
    }

    const auto style = WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;
    RECT rect = { 0, 0, static_cast<LONG>(setup_.clientWidth), static_cast<LONG>(setup_.clientHeight) };
    AdjustWindowRect(&rect, style, false);

    const auto windowWidth = rect.right - rect.left;
    const auto windowHeight = rect.bottom - rect.top;
    const auto windowHandle = CreateWindow(wc.lpszClassName, wc.lpszClassName, style, CW_USEDEFAULT, CW_USEDEFAULT,
        windowWidth, windowHeight, NULL, NULL, inst, NULL);

    if (windowHandle == NULL)
    {
        GF_LOG_ERROR("Game window creation error. Failed to create window.");
        throw WindowsException("Window class creation failed.");
    }

    GF_LOG_INFO("Game window creation succeeded");

    window = std::make_shared<Window>(windowHandle, setup_.clientWidth, setup_.clientHeight);
    setFrameRate(setup_.frameRate);

    for (int i = 0; i < MAX_FRAME_RATE; ++i)
    {
        latestDeltaTimes_s_.push_back(frameTime_us_ * 0.000001f);
    }

    // Resource
    fileSystem.startup("asset");
    resourceManager.startup();

    // Audio
    audioManager.startup();

    // Render
    renderSystem.startup(*window, 2);

    // Scene
    sceneAppContext.startup();
    debugDraw.startup();

    const auto aspect = static_cast<float>(setup_.clientWidth) / setup_.clientHeight;
    renderCamera.view = Matrix44::IDENTITY;
    renderCamera.proj = makePerspectiveMatrix(radian(45), aspect, 0.01f, 1000);
    renderCamera.viewport = renderSystem.fullyViewport();

    // Physics

    // Input
    inputDevice.startup(window);

    GF_LOG_INFO("GameFriends initialization succeeded.");

    /* Game loop */

    window->show(true);
    bool firstTick = true;
    auto preTime_us = std::chrono::high_resolution_clock::now();

    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            if (firstTick)
            {
                beginPlay();
                firstTick = false;
            }

            auto curTime_us = std::chrono::high_resolution_clock::now();
            auto deltaTime_us = std::chrono::duration_cast<std::chrono::microseconds>(curTime_us - preTime_us).count();
            if (deltaTime_us < frameTime_us_)
            {
                const auto sleepTime_us = frameTime_us_ - deltaTime_us;
                Sleep(static_cast<DWORD>(sleepTime_us * 0.001));
                curTime_us = std::chrono::high_resolution_clock::now();
                deltaTime_us = std::chrono::duration_cast<std::chrono::microseconds>(curTime_us - preTime_us).count();
            }
            preTime_us = curTime_us;

            auto dt_s = deltaTime_us * 0.000001f;
            latestDeltaTimes_s_.pop_front();
            latestDeltaTimes_s_.push_back(dt_s);

            tick(dt_s);
            physicsWorld.stepSimulation(dt_s);
            renderWorld.draw(renderCamera);
            debugDraw.drawDebugs(renderCamera);

            sceneAppContext.executeCommandsAndPresent();
            inputDevice.nextFrame();
        }
    }

    endPlay();
    window->show(false);

    /* Finalize */
    GF_LOG_INFO("Application will be shutdown.");

    shutdown();

    // Input
    inputDevice.shutdown();

    // Physics
    physicsWorld.setDebugDrawer(nullptr);
    physicsWorld.clearWorld();

    // Scene
    renderWorld.clearEntities();
    debugDraw.shutdown();
    sceneAppContext.shutdown();

    // Render
    renderSystem.shutdown();

    // Audio
    audioManager.shutdown();

    // Resource
    resourceManager.shutdown();
    fileSystem.shutdown();

    window.reset();

    logManager.shutdown();
}

void Application::quit()
{
    PostQuitMessage(0);
}

void Application::setupApp(const AppSetup& setup)
{
    setup_ = setup;
}

void Application::setFrameRate(size_t fps)
{
    if (fps > MAX_FRAME_RATE)
    {
        fps = MAX_FRAME_RATE;
    }
    frameTime_us_ = 1000000 / fps;

    GF_LOG_DEBUG("Frame rate set as {}", fps);
}

size_t Application::latestFrameRate(size_t collectFrames)
{
    if (collectFrames > MAX_FRAME_RATE)
    {
        collectFrames = MAX_FRAME_RATE;
    }

    float deltaTime_s = 0;
    for (size_t i = 0; i < collectFrames; ++i)
    {
        deltaTime_s += latestDeltaTimes_s_[i];
    }

    return static_cast<size_t>(collectFrames / deltaTime_s);
}

GF_NAMESPACE_END