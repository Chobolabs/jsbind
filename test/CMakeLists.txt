if(JSBIND_EMSCRIPTEN)
    add_executable(jsbind-test test_main.cpp)
else()
    message(FATAL_ERROR "JSBind: Unsupported jsbind js engine")
endif()

target_link_libraries(jsbind-test jsbind-testlib doctest)

if(JSBIND_EMSCRIPTEN)
    set_target_properties(jsbind-test PROPERTIES
        LINK_FLAGS "--bind")
    add_test(NAME jsbind COMMAND node jsbind-test.js)
else()
    message(FATAL_ERROR "JSBind: Unsupported jsbind js engine")
endif()

