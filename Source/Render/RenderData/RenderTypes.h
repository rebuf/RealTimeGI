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






// Stages types supported by RenderShader
enum class ERenderShaderStage : uint32_t
{
	// Invalid.
	None = 0,

	// Vertex Shader Stage.
	Vertex = 1,

	// Fragment Shader Stage.
	Fragment = 2,

	// Geoemtry Shader Stage.
	Geometry = 4,

	// All Shader Stages.
	AllStages = Vertex | Fragment | Geometry
};


// ERenderShaderStage Bit Operators
inline ERenderShaderStage operator&(const ERenderShaderStage& a, const ERenderShaderStage& b) { return static_cast<ERenderShaderStage>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b)); }
inline ERenderShaderStage operator|(const ERenderShaderStage& a, const ERenderShaderStage& b) { return static_cast<ERenderShaderStage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b)); }
inline ERenderShaderStage operator~(const ERenderShaderStage& a) { return static_cast<ERenderShaderStage>(~static_cast<uint32_t>(a)); }





// Types defining the domains the shader resides in
enum class ERenderShaderDomain
{
	// Invalid
	None = 0,

	// Rendre a Mesh.
	Mesh,

	// Render a triangle that cover entire screen
	Screen
};









// Shader Input Type.
enum class ERenderShaderInputType
{
	// Invalid
	None = 0,

	// An image and its smapler.
	ImageSampler,

	// Uniform Buffer.
	Uniform,

	// Dynamic Uniform that can be offsetted dynamically.
	DynamicUniform
};




// 
enum class ERenderMaterialType
{
	// Invalid
	None = 0,
	Opaque,
	Masked,
	Translucent
};




// Blending Factors.
enum class ERenderBlendFactor
{
	Zero = 0,
	One = 1,

	SrcColor = 2,
	OneMinusSrcColor = 3,
	DstColor = 4,
	OneMinusDstColor = 5,

	SrcAlpha = 6,
	OneMinusSrcAlpha = 7,
	DstAlpha = 8,
	OneMinusDstAlpha = 9,

	ConstantColor = 10,
	OneMinusConstantColor = 11,
	ConstantAlpha = 12,
	OneMinusConstantAlpha = 13,
	SrcAlphaSaturate = 14,

	Src1Color = 15,
	OneMinusSrc1Color = 16,
	SRC1_ALPHA = 17,
	OneMinusSrc1Alpha = 18
};





// Blending Operations.
enum class ERenderBlendOp
{
	Add = 0,
	Subtract = 1,
	ReverseSubtract = 2,
	Min = 3,
	Max = 4
};

