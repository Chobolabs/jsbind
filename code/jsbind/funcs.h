// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#pragma once

namespace jsbind
{

// compatibility
void initialize();
void deinitialize();

void enter_context();
void exit_context();

void run_script(const char* src, const char* fname = nullptr);

}