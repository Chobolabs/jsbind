// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#pragma once

#define JSBIND_VERISON_MAJOR 1
#define JSBIND_VERISON_MINOR 0
#define JSBIND_VERISON_SUB_MINOR 0

/// The library's version.
///
/// It is composed of `major*10000 + minor*100 + subminor`.
/// For example, version 2.31.11 would be 23111.
#define JSBIND_VERSION (JSBIND_VERSION_MAJOR*10000 + JSBIND_VERSION_MINOR*100 + JSBIND_VERSION_SUB_MINOR)
