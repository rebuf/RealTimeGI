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



#include "Core/Core.h"
#include "glm/vec2.hpp"



class VKIDevice;
class Image2D;
class VKIImage;
class VKIImageView;
class VKISampler;





// RenderImage:
//   - Mange image render data.
//
class RenderImage
{
public:
	// Construct.
	RenderImage();

	// Destruct..
	~RenderImage();

	// Set the image data.
	void SetData(Image2D* img);

	// Create Image View.
	void CreateView();

	// Create Sampler for the image.
	void CreateSampler();

	// Return this image view.
	VKIImageView* GetView() { return mView.get(); }

	// Return this image sampler.
	VKISampler* GetSampler() { return mSampler.get(); }


private:
	// Destroy Staging after it was submited & finished.
	void DestroyStaging();

private:
	// Device used to create this render image.
	VKIDevice* mDevice;

	// The Vulkan Image
	UniquePtr<VKIImage> mImage;

	// The Vulkan Image View
	UniquePtr<VKIImageView> mView;

	// The Vulkan Sampler.
	UniquePtr<VKISampler> mSampler;

	// A Staging buffer used to update the image.
	class VKIBuffer* mImgBuffer;
};

