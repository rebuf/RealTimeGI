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




#include "Core.h"
#include "glm/vec2.hpp"


#include <string>




// Image Formats.
enum class EImageFormat : uint32_t
{
	// Invalid Type.
	None,

	//
	R,
	RGB,
	RGBA
};



// Return the pixel info associated with image format.
extern void GetImgPixelInfo(EImageFormat format, int32_t& outBPP);




// Image2DData:
//    - 
//
class Image2DData
{
public:
	// Construct.
	Image2DData()
		: mData(nullptr)
		, mSize(0)
	{

	}

	// Destruct.
	~Image2DData()
	{
		free(mData);
	}

	// Allocate new image data.
	// @param size: the size in bytes.
	inline void Allocate(uint32_t size)
	{
		CHECK(mData == nullptr && "Trying to reallocate before data get freed.");
		mSize = size;
		mData = (uint8_t*)malloc(size);
	}

	// Copy data into this image data.
	void CopyData(uint32_t offset, uint32_t size, uint8_t* src)
	{
		memcpy(mData + offset, src, size);
	}

	// Free allocated image data.
	inline void Reset()
	{
		mSize = 0;
		free(mData);
	}

	// Return the image data.
	inline uint8_t* GetData() { return mData; }
	inline const uint8_t* GetData() const { return mData; }

	// Return the allocated image data size in bytes.
	inline uint32_t GetSize() { return mSize; }

	// Return true if the image data is allocated and valid.
	inline bool IsValid() { return mData != nullptr; }

private:
	// The image data.
	uint8_t* mData;

	// Size in bytes.
	uint32_t mSize;
};




// Image2D:
//	 -  
//
class Image2D
{
public:
	// Construct.
	Image2D();

	// Destruct.
	~Image2D();

	// Allocate dynamic image data of format and size.
	void Allocate(EImageFormat format, const glm::ivec2& size);

	// Reset iamge and clear its data.
	void Reset();

	// Load image from file.
	bool LoadImage(const std::string& imgFile);

	// Save image to a file.
	bool SaveImage(const std::string& imgFile);

	// Return the image size.
	inline const glm::ivec2& GetSize() { return mSize; }
	

private:
	// The image size.
	glm::ivec2 mSize;

	// The image format.
	EImageFormat mFormat;

	// The Image Data.
	Image2DData data;
};

