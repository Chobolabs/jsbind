# jsbind
# Copyright (c) 2019 Chobolabs Inc.
# http://www.chobolabs.com/
#
# Distributed under the MIT Software License
# See accompanying file LICENSE.txt or copy at
# http://opensource.org/licenses/MIT
#
if(NOT JBIND_TESTLIB_DOCTEST_TARGET)
    message(FATAL_ERROR "JSBind: Missing doctest target for jsbind-testlib")
endif()

set(sources
    test.cpp
    person.cpp
    person.h
    testclass.cpp
    testclass.h
    pods.cpp
    pods.h
)

# if((NOT JSBIND_JSC) OR JSBIND_JSC_TYPED_ARRAYS OR JSBIND_JSC_PATCH_TYPED_ARRAYS)
#     set(sources ${sources} shared_memory_extension_test.cpp)
# endif()

add_library(jsbind-testlib STATIC
    ${sources}
)

target_include_directories(jsbind-testlib INTERFACE .)

target_link_libraries(jsbind-testlib
    jsbind
    ${JBIND_TESTLIB_DOCTEST_TARGET}
)
