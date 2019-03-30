// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#include <include/cef_app.h>
#include <include/cef_client.h>
#include <include/wrapper/cef_message_router.h>
#include <include/wrapper/cef_resource_manager.h>

#include <thread> // so we can sleep in the main loop
#include <atomic>

#if defined(CEF_USE_SANDBOX) && defined(_WIN32)
#   include <include/cef_sandbox_win.h>
#endif

// defined in test_cef_renderer_app.cpp
extern CefRefPtr<CefApp> CreateRendererApp();

const char* HTML = R"demohtml(
<!DOCTYPE html>
<html>
<script>
    Module.htmlLoaded(); // notify that it's safe to exit
</script>
</html>
)demohtml";

const char* URL = "https://jsbind/test";

std::atomic_bool safeToExit;
int testResult;

void SetupResourceManagerOnIOThread(CefRefPtr<CefResourceManager> resourceManager)
{
    if (!CefCurrentlyOn(TID_IO))
    {
        CefPostTask(TID_IO, base::Bind(SetupResourceManagerOnIOThread, resourceManager));
        return;
    }

    resourceManager->AddContentProvider(URL, HTML, "text/html", 10, std::string());
}

class HeadlessClient : public CefClient, public CefRequestHandler, public CefRenderHandler
{
public:
    HeadlessClient()
        : m_resourceManager(new CefResourceManager)
    {
        SetupResourceManagerOnIOThread(m_resourceManager);
    }

    virtual CefRefPtr<CefRequestHandler> GetRequestHandler() override { return this; }
    virtual CefRefPtr<CefRenderHandler> GetRenderHandler() override { return this; }

    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
        CefProcessId source_process,
        CefRefPtr<CefProcessMessage> message) override
    {
        if (message->GetName() == "testing_done")
        {
            testResult = message->GetArgumentList()->GetInt(0);
            safeToExit = true;
            return true;
        }
        return false;
    }

    /////////////////////////////////////
    // request handler

    virtual cef_return_value_t OnBeforeResourceLoad(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefRequest> request,
        CefRefPtr<CefRequestCallback> callback) override
    {
        return m_resourceManager->OnBeforeResourceLoad(browser, frame, request, callback);
    }

    virtual CefRefPtr<CefResourceHandler> GetResourceHandler(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefRequest> request) override
    {
        return m_resourceManager->GetResourceHandler(browser, frame, request);
    }

    /////////////////////////////////////
    // render handler
    virtual void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) override
    {
        rect = CefRect(0, 0, 200, 200); // some random rect
    }

    virtual void OnPaint(CefRefPtr<CefBrowser> browser,
        PaintElementType type,
        const RectList& dirtyRects,
        const void* buffer,
        int width,
        int height) override
    {} // noop. nothing to do


private:
    CefRefPtr<CefResourceManager> m_resourceManager;

    IMPLEMENT_REFCOUNTING(HeadlessClient);
    DISALLOW_COPY_AND_ASSIGN(HeadlessClient);
};

int main(int argc, char* argv[])
{
    CefRefPtr<CefCommandLine> commandLine = CefCommandLine::CreateCommandLine();
#if defined(_WIN32)
    CefEnableHighDPISupport();
    CefMainArgs args(GetModuleHandle(NULL));
    commandLine->InitFromString(GetCommandLineW());
#else
    CefMainArgs args(argc, argv);
    commandLine->InitFromArgv(argc, argv);
#endif

    void* windowsSandboxInfo = NULL;

#if defined(CEF_USE_SANDBOX) && defined(_WIN32)
    // Manage the life span of the sandbox information object. This is necessary
    // for sandbox support on Windows. See cef_sandbox_win.h for complete details.
    CefScopedSandboxInfo scopedSandbox;
    windowsSandboxInfo = scopedSandbox.sandbox_info();
#endif

    CefRefPtr<CefApp> app = nullptr;
    if (commandLine->GetSwitchValue("type") == "renderer") {
        app = CreateRendererApp();
    }
    int result = CefExecuteProcess(args, app, windowsSandboxInfo);
    if (result >= 0)
    {
        // child process completed
        return result;
    }

    CefSettings settings;
    settings.windowless_rendering_enabled = 1;
#if !defined(CEF_USE_SANDBOX)
    settings.no_sandbox = true;
#endif

    CefInitialize(args, settings, nullptr, windowsSandboxInfo);

    CefWindowInfo windowInfo;
    windowInfo.SetAsWindowless(0);

    CefBrowserSettings browserSettings;
    auto browser = CefBrowserHost::CreateBrowserSync(windowInfo, new HeadlessClient, URL, browserSettings, nullptr);

    // main loop
    while (!safeToExit) {
        CefDoMessageLoopWork();
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
    }

    browser = nullptr;
    CefShutdown();

    return testResult;
}