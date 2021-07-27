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




#include "Image2D.h"
#include "Render/RenderData/RenderImage.h"


// Disable compilers warnings caused by stb_image.
#pragma warning(disable : 26451)
#pragma warning(disable : 26495)
#pragma warning(disable : 6308)
#pragma warning(disable : 6262)
#pragma warning(disable : 6308)
#pragma warning(disable : 6387)
#pragma warning(disable : 26819)

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"





void GetImgPixelInfo(EImageFormat format, int32_t& outBPP)
{
	switch (format)
	{
	case EImageFormat::R:
		outBPP = 8;
		break;

	case EImageFormat::RGB:
		outBPP = 24;
		break;

	case EImageFormat::RGBA:
		outBPP = 32;
		break;

	default:
		CHECK(0 && "Unsupported Format.");
		break;
	}

}


Image2D::Image2D()
	: mFormat(EImageFormat::None)
	, mIsSRGB(true)
{

}


Image2D::~Image2D()
{

}


void Image2D::Allocate(EImageFormat format, const glm::ivec2& size)
{
	mFormat = format;
	mSize = size;

	int32_t BPP = 0;
	GetImgPixelInfo(format, BPP);

	int32_t sizeBytes = (BPP >> 3) * size.x * size.y;
	mData.Allocate(sizeBytes);
}


void Image2D::Reset()
{
	// Free data.
	mData.Reset();


}


bool Image2D::LoadImage(const std::string& imgFile)
{
	// Load Image using stb...
	int imgWidth, imgHeight, imgNumChannels;
	stbi_uc* pixels = stbi_load(imgFile.c_str(), &imgWidth, &imgHeight, &imgNumChannels, STBI_rgb_alpha);

	// Failed to Load?
	if (!pixels)
	{
		LOGE("Failed to load image file(%s)", imgFile.c_str());
		return false;
	}


	// Set format based on the number of channels
	switch (imgNumChannels)
	{
	case 1: mFormat = EImageFormat::R; break;
	case 2: mFormat = EImageFormat::RGB; break;
	case 3: mFormat = EImageFormat::RGBA; break;
	}


	// Allocate & Copy
	Allocate(mFormat, glm::ivec2(imgWidth, imgHeight));
	mData.CopyData(0, mData.GetSize(), pixels);

	// Free Loaded Image.
	stbi_image_free(pixels);

	return true;
}


bool Image2D::SaveImage(const std::string& imgFile)
{
	CHECK(0 && "Not supported yet.");
	return false;
}


void Image2D::UpdateRenderImage()
{
	mRenderImage = UniquePtr<RenderImage>(new RenderImage());
	mRenderImage->SetData(this);
	mRenderImage->CreateView();
	mRenderImage->CreateSampler();
}
