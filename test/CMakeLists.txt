if(JSBIND_EMSCRIPTEN OR JSBIND_JSC OR JSBIND_V8)
    add_executable(jsbind-test test_main.cpp)
elseif(JSBIND_NODE)
    add_library(jsbind-test SHARED test_node_main.cpp)
    target_compile_definitions(jsbind-test PRIVATE
        -DNODE_GYP_MODULE_NAME="jsbind-test"
        -DBUILDING_V8_SHARED=1
        -DBUILDING_UV_SHARED=1
        -DBUILDING_NODE_EXTENSION
    )

    if(APPLE)
        set_target_properties(jsbind-test PROPERTIES LINK_FLAGS "-undefined dynamic_lookup \
            -Wl,-no_pie \
            -Wl,-search_paths_first"
        )
    endif()

    set_target_properties(jsbind-test PROPERTIES PREFIX "" SUFFIX ".node")
elseif(JSBIND_CEF)
    add_executable(jsbind-test
        test_cef_main.cpp
        test_cef_renderer_app.cpp
    )
else()
    message(FATAL_ERROR "JSBind: Unsupported jsbind js engine")
endif()

target_link_libraries(jsbind-test jsbind-testlib doctest)

if(JSBIND_EMSCRIPTEN)
    set_target_properties(jsbind-test PROPERTIES
        LINK_FLAGS "--bind")
    add_test(NAME jsbind COMMAND node jsbind-test.js)
elseif(JSBIND_NODE)
    if (MSVC)
        set(outputDir ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
    else()
        set(outputDir ${CMAKE_CURRENT_BINARY_DIR})
    endif()

    file(GENERATE
        OUTPUT ${outputDir}/jsbind-node-test.js
        CONTENT "Module = require('./jsbind-test');\nModule.run();"
    )
    add_test(NAME jsbind COMMAND node ${outputDir}/jsbind-node-test.js)
elseif(JSBIND_CEF OR JSBIND_JSC OR JSBIND_V8)
    add_test(NAME jsbind COMMAND $<TARGET_FILE:jsbind-test>)
else()
    message(FATAL_ERROR "JSBind: Unsupported jsbind js engine")
endif()

