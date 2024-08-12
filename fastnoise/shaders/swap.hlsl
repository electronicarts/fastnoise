///////////////////////////////////////////////////////////////////////////////
//               FastNoise - F.A.S.T. Sampling Implementation                //
//         Copyright (c) 2023 Electronic Arts Inc. All rights reserved.      //
///////////////////////////////////////////////////////////////////////////////

struct FilterType
{
    static const int Box = 0;
    static const int Gaussian = 1;
    static const int Binomial = 2;
    static const int Exponential = 3;
    static const int WeightedExponential = 4;
};

struct SampleSpace
{
    static const int Real = 0;
    static const int Circle = 1;
    static const int Vector2 = 2;
    static const int Vector3 = 3;
    static const int Vector4 = 4;
    static const int Sphere = 5;
};

struct SampleDistribution
{
    static const int Uniform1D = 0;
    static const int Gauss1D = 1;
    static const int Tent1D = 2;
    static const int Uniform2D = 3;
    static const int Uniform3D = 4;
    static const int Uniform4D = 5;
    static const int UniformSphere = 6;
    static const int UniformHemisphere = 7;
    static const int CosineHemisphere = 8;
};

struct Struct_DataStruct
{
    uint initialized;
    uint iterationSum;
    uint swaps;
};

struct Struct__SwapCB
{
    uint Iteration;
    uint3 TextureSize;
    uint4 key;
    uint scrambleBits;
    uint swapSuppression;
    float2 _padding0;
};

Texture3D<float> LossTexture : register(t0);
RWTexture3D<float4> SampleTexture : register(u0);
RWTexture3D<float4> SwapDebug : register(u1);
RWStructuredBuffer<Struct_DataStruct> Data : register(u2);
ConstantBuffer<Struct__SwapCB> _SwapCB : register(b0);

#line 1


#include "fastnoise.hlsl"

[numthreads(8, 8, 1)]
#line 5
void Swap(uint3 DTid : SV_DispatchThreadID)
{
	// 1. Total loss for the swap is sum of loss texture at source and destination
	uint3 index = DTid;
	uint3 otherIndex = getOtherIndex(index, _SwapCB.key, _SwapCB.scrambleBits);

	float loss = LossTexture[index] + LossTexture[otherIndex];

	// 2. Only one out of each pair does the swap, so check if we have the lower index
	int3 textureSize = _SwapCB.TextureSize;
	uint3 flatten = uint3(textureSize.y * textureSize.z, textureSize.z, 1);
	bool lesser = dot(flatten, index) < dot(flatten, otherIndex);

	// 3. Only do swap a fraction of the time, this helps convergence in the early iterations
	// TODO: We should adjust this based on the number of swaps - if it's above a threshold, then don't execute the swap


	uint iteration = _SwapCB.Iteration;
	uint randomSeed = wang_hash_init(DTid + iteration);
	uint randomValue = wang_hash_uint(randomSeed);
	uint swapSuppression = _SwapCB.swapSuppression;
	bool swapCheck = (randomValue % swapSuppression) == 0;

	if (lesser && loss < 0 && swapCheck)
	{
		float4 value = SampleTexture[index];
		float4 otherValue = SampleTexture[otherIndex];
		SampleTexture[index] = otherValue;
		SampleTexture[otherIndex] = value;

		uint oldSwaps;
		InterlockedAdd(Data[0].swaps, 1, oldSwaps);

	}
	
	// Output debugging info to a texture
	// Should have equal numbers of red and green pixels
	//float4 debugValue = 0.0f;
	//if (loss < 0) {
	//	debugValue = float4(lesser, 1 - lesser, -loss, 1.0f);
	//}
	//float4 debugValue = float4(loss, loss, loss, 1.0f);
	//SwapDebug[index] = debugValue;

	// DEBUG: check that this defines an involution
	//uint2 otherOtherIndex = scramble(scramble(otherIndex, a) ^ uint2(0, 1), b);
	//SwapDebug[index] = float4(otherOtherIndex, 0.0f, 1.0f);

}
