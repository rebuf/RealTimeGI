// Copyright (c) 2021 Ammar Herzallah
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.




#pragma once



// Defaults Defs...
#ifndef BUILD_DEBUG
#define BUILD_DEBUG 0
#endif

#ifndef BUILD_RELEASE
#define BUILD_RELEASE 0
#endif

#ifndef BUILD_WIN
#define BUILD_WIN 0
#endif





// PATH TO THE RESROUCE DIRECTORY
#ifdef PROJ_PATH
#define RESOURCES_DIRECTORY PROJ_PATH "Resources/"
#else
#define RESOURCES_DIRECTORY "/Resources/"
#endif

#define SHADERS_DIRECTORY RESOURCES_DIRECTORY "Shaders/SPV/"



// Common Includes...
#include "CoreTypes.h"




extern void LOG_MSG(ELogType logType, const char* msg, ...);



#define LOGI(msg, ...) LOG_MSG(ELogType::Info, msg, __VA_ARGS__);
#define LOGW(msg, ...) LOG_MSG(ELogType::Warning, msg, __VA_ARGS__);
#define LOGE(msg, ...) LOG_MSG(ELogType::Error, msg, __VA_ARGS__);





// Asserts.
#if BUILD_DEBUG
#define DEBUG_BREAK() __debugbreak()
#define CHECK(x) if ((x) == 0) { LOGE("CHECK FAILED LINE(%d), FILE(%s)", __LINE__, __FILE__); DEBUG_BREAK(); exit(-2);  } 
#else
#define CHECK(x)
#endif




// Common Definitions.
#define SMALL_NUM 0.1e-5f

#define PI 3.14159265358f
#define TWO_PI 6.283185307179f
#define HALF_PI 1.57079632679f
#define ONE_OVER_PI 0.3183098861837f

#define INVALID_INDEX -1
#define INVALID_UINDEX 0xFFFFFFFFU


// Number of bounces to compute for global illumination.
extern uint32_t g_NumOfBounces;


// Align memory size to a Base alignment.
#define ALIGN_SIZE(SIZE, BASE) ((SIZE + BASE - 1) & ~(BASE - 1))






