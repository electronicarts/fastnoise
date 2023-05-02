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

struct Struct__InitCB
{
    uint4 key;
    uint Iteration;
    uint scrambleBits;
    uint rngSeed;
    int sampleDistribution;
};

RWTexture3D<float4> Texture : register(u0);
RWStructuredBuffer<Struct_DataStruct> Data : register(u1);
ConstantBuffer<Struct__InitCB> _cb : register(b0);

// https://www.shadertoy.com/view/MlVSzw
float inv_error_function(float x)
{
	const float ALPHA = 0.14f;
	const float INV_ALPHA = 1.0f / ALPHA;
	const float K = 2.0f / (3.14159265359f * ALPHA);

	float y = log(1.0f - x * x);
	float z = K + 0.5f * y;
	return sqrt(sqrt(z * z - y * INV_ALPHA) - z) * sign(x);
}

#include "fastnoise.hlsl"

[numthreads(8, 8, 1)]
void Init(uint3 DTid : SV_DispatchThreadID)
{

	// Set swap count to zero
	if (all(DTid == 0))
	{
		Data[0].swaps = 0;
	}

	// Beyond this point only do first-run initialization
	if (_cb.Iteration > 0)
		return;

	// Calculate based on the other index
	uint3 otherIndex = getOtherIndex(DTid, _cb.key, _cb.scrambleBits);

	// How many bits of each index to scramble
	uint3 bits = uint3(_cb.scrambleBits, _cb.scrambleBits, 0);
	uint3 mask = (1 << bits) - 1;
	uint3 shift = uint3(bits.y + bits.z, bits.z, 0);
	uint totalBits = bits.x + bits.y + bits.z;
	uint totalMask = (1 << totalBits) - 1;

	// Uniform histogram in each block, but scrambled with Wang hash
	uint3 lowIndex = otherIndex & mask;
	uint3 highIndex = otherIndex >> bits;
	uint lowBits = (lowIndex.x << shift.x) | (lowIndex.y << shift.y) | lowIndex.z;


	uint v = lowBits;
	uint rng = wang_hash_init(uint3(DTid.xy, _cb.rngSeed));

	// At this point we have v which is a random number of "totalBits" number of bits
	// We use this to generate a stratified sample
	float4 value = 0.0f;
	if (_cb.sampleDistribution == SampleDistribution::Uniform1D)
	{
		float f = (v + wang_hash_float01(rng)) / float(1 << totalBits);
		value = float4(f, f, f, 1.0f);
	}
	else if (_cb.sampleDistribution == SampleDistribution::Tent1D)
	{
		float u = (v + wang_hash_float01(rng)) / float(1 << totalBits);
		float f = 0.0f;
		if (u < 0.5f) 
		{
			f = 1.0f - 0.5f* sqrt(2.0f * u);
		}
		else
		{
			f = 0.5f * sqrt(2.0f - 2.0f * u);
		};
		value = float4(f, f, f, 1.0f);
	}
	else if (_cb.sampleDistribution == SampleDistribution::Gauss1D)
	{
		float u = (v + wang_hash_float01(rng)) / float(1 << totalBits);
		float f = inv_error_function(u * 2.0f - 1.0f) * 0.15f + 0.5f;
		value = float4(f, f, f, 1.0f);
	}
	else if (_cb.sampleDistribution == SampleDistribution::Uniform2D)
	{
		uint halfTotalBits = totalBits / 2;
		uint i = v >> halfTotalBits;
		uint j = v & ((1 << halfTotalBits)-1);
		float a = (i + wang_hash_float01(rng)) / float(1 << (totalBits - halfTotalBits));
		float b = (j + wang_hash_float01(rng)) / float(1 << halfTotalBits);
		value = float4(a, b, 0.0f, 1.0f);
	}
	else if (_cb.sampleDistribution == SampleDistribution::CosineHemisphere)
	{
		uint halfTotalBits = totalBits / 2;
		uint i = v >> halfTotalBits;
		uint j = v & ((1 << halfTotalBits) - 1);

		// Uniform sample
		float2 u = float2(i + wang_hash_float01(rng), j + wang_hash_float01(rng)) / float2(1 << (totalBits - halfTotalBits), 1 << halfTotalBits);
		float2 rTheta = squareToDiskPolar(2.0f * u - 1.0f);
		float3 w = float3(rTheta.x * float2(cos(rTheta.y), sin(rTheta.y)), sqrt(1.0f - rTheta.x*rTheta.x));

		// w is in the unit sphere, remap to [0,1] range for later storage in a texture
		value = float4(0.5f + 0.5f * w, 1.0f);
	}
	else if (_cb.sampleDistribution == SampleDistribution::UniformHemisphere)
	{
		uint halfTotalBits = totalBits / 2;
		uint i = v >> halfTotalBits;
		uint j = v & ((1 << halfTotalBits) - 1);

		// Uniform sample
		float2 u = float2(i + wang_hash_float01(rng), j + wang_hash_float01(rng)) / float2(1 << (totalBits - halfTotalBits), 1 << halfTotalBits);
		float2 rTheta = squareToDiskPolar(2.0f * u - 1.0f);
		float3 w = float3(rTheta.x * sqrt(2.0f - rTheta.x * rTheta.x) * float2(cos(rTheta.y), sin(rTheta.y)), 1.0f - rTheta.x * rTheta.x);

		// v is in the unit sphere, remap to [0,1]
		value = float4(0.5f + 0.5f *w, 1.0f);
	}
	else if (_cb.sampleDistribution == SampleDistribution::UniformSphere)
	{
		// Just as uniform hemisphere, but use one extra bit to specify which hemisphere we're on
		uint halfTotalBits = totalBits / 2;
		uint3 bits = uint3(halfTotalBits, totalBits - halfTotalBits - 1, 1);
		uint3 stratifiedIndex = splitBits(v, bits);

		// Uniform sample from hemisphere
		float2 u = (stratifiedIndex.xy + float2(wang_hash_float01(rng), wang_hash_float01(rng))) / float2(1 << bits.xy);
		float2 rTheta = squareToDiskPolar(2.0f * u - 1.0f);
		float3 w = float3(rTheta.x * sqrt(2.0f - rTheta.x * rTheta.x) * float2(cos(rTheta.y), sin(rTheta.y)), 1.0f - rTheta.x * rTheta.x);
		if (stratifiedIndex.z)
		{
			w.z = -w.z;
		}

		// v is in the unit sphere, remap to [0,1]
		value = float4(0.5f + 0.5f * w, 1.0f);
	}
	else if (_cb.sampleDistribution == SampleDistribution::Uniform3D)
	{
		uint oneThirdTotalBits = totalBits / 3;
		uint3 stratBits = uint3(oneThirdTotalBits, oneThirdTotalBits, totalBits - 2* oneThirdTotalBits);
		uint3 stratShift = uint3(stratBits.y + stratBits.z, stratBits.z, 0);
		uint3 base = (v >> stratShift) & ((1 << stratBits) - 1);
		float3 offset = float3(wang_hash_float01(rng), wang_hash_float01(rng), wang_hash_float01(rng));
		value = float4((base + offset) / (1 << stratBits), 1.0f);
	}
	else if (_cb.sampleDistribution == SampleDistribution::Uniform4D)
	{
		// TODO: distribute bits more fairly between components?
		uint oneQuarterTotalBits = totalBits / 4;
		uint4 stratBits = uint4(oneQuarterTotalBits, oneQuarterTotalBits, oneQuarterTotalBits, totalBits - 3 * oneQuarterTotalBits);
		uint4 stratShift = uint4(stratBits.y + stratBits.z + stratBits.w, stratBits.z + stratBits.w, stratBits.w, 0);
		uint4 base = (v >> stratShift) & ((1 << stratBits) - 1);
		float4 offset = float4(wang_hash_float01(rng), wang_hash_float01(rng), wang_hash_float01(rng), wang_hash_float01(rng));
		value = (base + offset) / (1 << stratBits);
	}


	Texture[DTid] = value;

	// This isn't really useful for anything, but it's a second example of how to write something the CPU can read.
	// The other example is iterationSum in process.hlsl
	if (DTid.x == 0 && DTid.y == 0)
		Data[0].initialized = true;
}
