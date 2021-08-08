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









// Volume Data for Irradiance Volume
struct IrradianceVolumeData
{
	// The start of the volume.
	vec3 Start;

	// The Extent of the volume.
	vec3 Extent;

	// The number of probes in each axis of the volume.
	ivec3 Count;

	//
	vec3 GridSize;

	//
	float GridLen;
};




ivec3 GetGridCoord(in vec3 Pos, in IrradianceVolumeData IrVolume)
{
	return ivec3((Pos - IrVolume.Start + IrVolume.GridSize) / IrVolume.GridSize) - 1;
}



int GetProbeIndex(ivec3 GridCoord, in IrradianceVolumeData IrVolume)
{
	return GridCoord.x + GridCoord.y * IrVolume.Count.x + GridCoord.z * (IrVolume.Count.x * IrVolume.Count.y);
}


vec3 GetProbePos(ivec3 GridCoord, in IrradianceVolumeData IrVolume)
{
	return vec3(GridCoord) * IrVolume.GridSize + IrVolume.GridSize * 0.5 + IrVolume.Start;
}



float SampleIrOcclusion(vec3 v, float ld, float layer, in samplerCubeArray Radiance) 
{ 
	float bias = 0.016; 
	
	float offset = 2.5;
	float numSamples = 3.0;
	float ShadowValue = 0.0;
	
	for(float x = -offset; x < offset; x += offset / (numSamples * 0.5)) 
	{ 
		for(float y = -offset; y < offset; y += offset / (numSamples * 0.5))
		{
			for(float z = -offset; z < offset; z += offset / (numSamples * 0.5)) 
			{
				float s_depth = texture(Radiance, vec4(v + vec3(x, y, z), layer)).a; 
				ShadowValue += s_depth > (ld - bias) ? 1.0 : 0.0;
			} 
		} 
	}

	ShadowValue /= 27.0;

	return 0.0;
}


vec4 SampleIrVolumeLayer(in ivec3 GridCoord, out vec3 Probe0Pos, in SurfaceData Surface, 
	in IrradianceVolumeData IrVolume, in samplerCubeArray Irradiance, in samplerCubeArray Radiance)
{
	Probe0Pos = GetProbePos(GridCoord, IrVolume);
	GridCoord = clamp(GridCoord, ivec3(0), IrVolume.Count - 1);
	int Probe0Index = GetProbeIndex(GridCoord, IrVolume);
	float SampleRadius = IrVolume.GridLen * 2.0;
	vec3 Sample0 = LightProbeSampleRay(Probe0Pos, SampleRadius, Surface.P, Surface.N);
	vec4 Irradiance0 = texture(Irradiance, vec4(Sample0, Probe0Index));
	
	vec3 V = Surface.P - Probe0Pos;
	float Dist = length(V);
	float Occlusion = SampleIrOcclusion(V, Dist * 0.001, Probe0Index, Radiance);
	
	if (dot(V, Surface.N) > 0.0)
		Occlusion *= abs(dot(V, Surface.N) * 0.05);

	return vec4(Irradiance0.rgb, 1.0);
}



vec4 SampleIrradianceVolume(in ivec3 GridCoord, in vec3 DiffCoord, out vec3 Probe0Pos, in SurfaceData Surface, 
	in IrradianceVolumeData IrVolume, in samplerCubeArray Irradiance, in samplerCubeArray Radiance)
{
	ivec3 Probe0GridCoord = GridCoord + ivec3(0,           0,           DiffCoord.z);
	ivec3 Probe1GridCoord = GridCoord + ivec3(DiffCoord.x, 0,           DiffCoord.z);
	ivec3 Probe2GridCoord = GridCoord + ivec3(0,           DiffCoord.y, DiffCoord.z);
	ivec3 Probe3GridCoord = GridCoord + ivec3(DiffCoord.x, DiffCoord.y, DiffCoord.z);


	vec3 Probe1Pos, Probe2Pos, Probe3Pos;
	vec4 Irradiance0 = SampleIrVolumeLayer(Probe0GridCoord, Probe0Pos, Surface, IrVolume, Irradiance, Radiance);
	vec4 Irradiance1 = SampleIrVolumeLayer(Probe1GridCoord, Probe1Pos, Surface, IrVolume, Irradiance, Radiance);
	vec4 Irradiance2 = SampleIrVolumeLayer(Probe2GridCoord, Probe2Pos, Surface, IrVolume, Irradiance, Radiance);
	vec4 Irradiance3 = SampleIrVolumeLayer(Probe3GridCoord, Probe3Pos, Surface, IrVolume, Irradiance, Radiance);

	vec2 Delta = Probe3Pos.xy - Probe0Pos.xy;
	vec2 Alpha = (Surface.P.xy - Probe0Pos.xy) / Delta;

	vec4 IrLerp0 = mix(Irradiance0, Irradiance1, Alpha.x);
	vec4 IrLerp1 = mix(Irradiance2, Irradiance3, Alpha.x);
	vec4 IrValue  = mix(IrLerp0, IrLerp1, Alpha.y);

	return IrValue;
}
