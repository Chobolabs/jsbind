// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#include <include/cef_app.h>

#include <jsbind.h>
#include <testlib.h>

#define DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

void onHtmlLoaded();

JSBIND_BINDINGS(Tests)
{
    jsbind::function("htmlLoaded", onHtmlLoaded);
}

namespace
{

int testResult = 416; // some value which is unlikely to get from doctest

class RendererApp : public CefApp, public CefRenderProcessHandler
{
public:
    RendererApp() = default;

    CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() { return this; }

    void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> /*frame*/, CefRefPtr<CefV8Context> /*context*/) override
    {
        jsbind::initialize();

        jsbind::test::jsbind_init_tests();
        doctest::Context context;
        testResult = context.run();
        jsbind::test::jsbind_deinit_tests();
    }

    void OnContextReleased(CefRefPtr<CefBrowser> /*browser*/, CefRefPtr<CefFrame> /*frame*/, CefRefPtr<CefV8Context> /*context*/) override
    {
        jsbind::deinitialize();
    }

private:
    IMPLEMENT_REFCOUNTING(RendererApp);
    DISALLOW_COPY_AND_ASSIGN(RendererApp);
};

}

void onHtmlLoaded()
{
    auto msg = CefProcessMessage::Create("testing_done");
    msg->GetArgumentList()->SetInt(0, testResult);
    CefV8Context::GetCurrentContext()->GetBrowser()->SendProcessMessage(PID_BROWSER, msg);
}

CefRefPtr<CefApp> CreateRendererApp()
{
    return new RendererApp;
}
