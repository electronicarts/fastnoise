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

struct Struct__LossCB
{
    int sampleSpace;
    float separateWeight;
    uint separate;
    float _padding0;
    uint4 key;
    uint scrambleBits;
    uint3 TextureSize;
    int3 filterMin;
    float _padding1;
    int3 filterMax;
    float _padding2;
    int3 filterOffset;
};

RWTexture3D<float> LossTexture : register(u0);
Buffer<float> Filter : register(t0);
Texture3D<float4> SampleTexture : register(t1);
ConstantBuffer<Struct__LossCB> _cb : register(b0);


#include "fastnoise.hlsl"

// Evaluate the two-point function
float K2(float4 x, float4 y)
{
	float K = 0.0f;
	if (_cb.sampleSpace == SampleSpace::Real)
	{
		K = -abs(x.x - y.x);
	}
	else if (_cb.sampleSpace == SampleSpace::Circle)
	{
		K = -min(abs(x.x - y.x), min(abs(x.x - y.x + 1.0f), abs(x.x - y.x - 1.0f)));
	}
	else if (_cb.sampleSpace == SampleSpace::Vector2)
	{
		K = -length(x.xy - y.xy);
	}
	else if (_cb.sampleSpace == SampleSpace::Vector3)
	{
		K = -length(x.xyz - y.xyz);
	}
	else if (_cb.sampleSpace == SampleSpace::Vector4)
	{
		K = -length(x - y);
	}
	else if (_cb.sampleSpace == SampleSpace::Sphere)
	{
		K = -acos(saturate(dot(2*x.xyz-1, 2*y.xyz-1)));
	}
	
	return K;
}

float combineFilter(int3 index, float filterX, float filterY, float filterZ)
{
	float F = 0.0f;
	if (_cb.separate)
	{
		if (index.z == 0)
		{
			F += filterX * filterY* _cb.separateWeight;
		}
		if (index.x == 0 && index.y == 0)
		{
			F += filterZ* (1.0f - _cb.separateWeight);
		}
	}
	else 
	{
		F = filterX * filterY * filterZ;
	}
	return F;
}

// Currently 3D box filter
float doubledFilter(int3 i, int3 filterMin, int3 filterMax, int3 filterOffset)
{
	float3 filter = 0.0f;
	if (i.x >= filterMin.x && i.x <= filterMax.x)
	{
		filter.x = Filter[i.x + filterOffset.x];
	}
	if (i.y >= filterMin.y && i.y <= filterMax.y)
	{
		filter.y = Filter[i.y + filterOffset.y];
	}
	if (i.z >= filterMin.z && i.z <= filterMax.z)
	{
		filter.z = Filter[i.z + filterOffset.z];
	}
	return combineFilter(i, filter.x, filter.y, filter.z);
}

[numthreads(8, 8, 1)]
void Loss(uint3 DTid : SV_DispatchThreadID)
{
	int3 index = DTid;
	float4 currentValue = SampleTexture[index];

	int3 otherIndex = getOtherIndex(index, _cb.key, _cb.scrambleBits);
	float4 otherValue = SampleTexture[otherIndex];

	uint3 textureSize = _cb.TextureSize;

	int3 filterMin = _cb.filterMin;
	int3 filterMax = _cb.filterMax;
	int3 filterOffset = _cb.filterOffset;

	float deltaLoss = 0.0f;

	for (int i = filterMin.x; i <= filterMax.x; ++i)
	{
		float filterX = Filter[i + filterOffset.x];

		for (int j = filterMin.y; j <= filterMax.y; ++j)
		{
			float filterY = Filter[j + filterOffset.y];

			for (int k = filterMin.z; k <= filterMax.z; ++k) {

				float filterZ = Filter[k + filterOffset.z];

				float F = combineFilter(int3(i, j, k), filterX, filterY, filterZ);

				float4 neighbourValue = SampleTexture[uint3(index + int3(i, j, k)) % textureSize];
				deltaLoss += F * (K2(otherValue, neighbourValue) - K2(currentValue, neighbourValue));

			}
		}
	}

	// Wrap indices
	int3 dij = min(abs(index - otherIndex), min(abs(index - otherIndex - int3(textureSize)), abs(index - otherIndex + int3(textureSize))));
	float Fij = doubledFilter(dij, filterMin, filterMax, filterOffset);
	float Fii = doubledFilter(int3(0, 0, 0), filterMin, filterMax, filterOffset);

	deltaLoss += (Fij - Fii) * K2(currentValue, otherValue);

	LossTexture[index] = deltaLoss;

}
