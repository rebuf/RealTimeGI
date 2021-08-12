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



#include "Core/Core.h"


#if BUILD_WIN
#include "Windows.h"
#endif




// Print Log from variadic argument.
#define PRINT_VAR_ARGS(MSG) \
  va_list Args; \
  va_start(Args, MSG); \
  PrintVarLog(MSG, Args); \
  va_end(Args);




uint32_t g_NumOfBounces = 2;







static void PrintVarLog(const char* msg, va_list args)
{
  vprintf(msg, args);
}


#if BUILD_WIN
static void PrintLocalTime()
{
  SYSTEMTIME sysTime;
  GetLocalTime(&sysTime);

  printf("%02d:%02d:%02d >> ", sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
}


static WORD GetDefaultConsoleColor(HANDLE Console)
{
  CONSOLE_SCREEN_BUFFER_INFO Info;
  GetConsoleScreenBufferInfo(Console, &Info);
  return Info.wAttributes;
}


void LOG_MSG(ELogType logType, const char* msg, ...)
{
  static HANDLE Console = GetStdHandle(STD_OUTPUT_HANDLE);
  static WORD DefaultTexColor = GetDefaultConsoleColor(Console);


  // Output Color...
  switch (logType)
  {
  case ELogType::Info:
    SetConsoleTextAttribute(Console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    break;

  case ELogType::Warning:
    SetConsoleTextAttribute(Console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    break;

  case ELogType::Error:
    SetConsoleTextAttribute(Console, FOREGROUND_RED | FOREGROUND_INTENSITY);
    break;
  }

  // Output Msg...
  PrintLocalTime();
  PRINT_VAR_ARGS(msg);
  printf("\n");

  // Reset To Default Console Color...
  SetConsoleTextAttribute(Console, DefaultTexColor);

}

#else
void LOG_MSG(ELogType logType, const char* msg, ...)
{
  printf(">> ");
  PRINT_VAR_ARGS(msg);
  printf("\n");
}
#endif



