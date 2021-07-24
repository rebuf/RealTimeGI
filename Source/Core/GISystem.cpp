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




#include "GISystem.h"



#if BUILD_WIN
#include "Windows.h"
#endif





std::string GISystem::GetFileExtension(const std::string& file, bool withDot)
{
	size_t idx = file.find_last_of('.');
	if (idx == std::string::npos)
		return "";

	return file.substr(idx);
}


std::string GISystem::NormalizePath(const std::string& path)
{
	std::string normPath;
	normPath.resize(path.size());

	for (size_t i = 0, c = 0; i < path.size(); ++i, ++c)
	{
		// Replace \\ -> /
		if (path[i] == '\\')
		{
			normPath[c] = '/';
		}
		else
		{
			normPath[c] = path[i];
		}


		// Remove // or /./
		if (normPath[c] == '/')
		{
			if (c > 1 && normPath[c - 1] == '/')
				c = c - 1;

			if (c > 2 && normPath[c - 1] == '.' && normPath[c - 2] == '/')
				c = c - 2;
		}
	}

	return normPath;
}


std::string GISystem::GetFileName(const std::string& file, bool withExt)
{
	size_t pathIdx = file.find_last_of({ '/', '\\' });

	if (pathIdx == std::string::npos)
		return "";

	if (!withExt)
	{
		size_t extIdx = file.find_last_of('.');

		if (extIdx != std::string::npos)
			return file.substr(pathIdx, (extIdx - pathIdx));
	}

	return file.substr(pathIdx);
}


std::string GISystem::GetDirectory(const std::string& file)
{
	size_t pathIdx = file.find_last_of({ '/', '\\' });

	if (pathIdx == std::string::npos)
		return file;

	return file.substr(0, pathIdx + 1);
}


void GISystem::Sleep(uint64_t ms)
{
	::Sleep(ms);
}
