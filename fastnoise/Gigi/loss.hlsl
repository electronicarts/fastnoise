/*$(ShaderResources)*/

#include "fastnoise.hlsl"

// Evaluate the two-point function
float K2(float4 x, float4 y)
{
	float K = 0.0f;
	if (/*$(Variable:sampleSpace)*/ == SampleSpace::Real)
	{
		K = -abs(x.x - y.x);
	}
	else if (/*$(Variable:sampleSpace)*/ == SampleSpace::Circle)
	{
		K = -min(abs(x.x - y.x), min(abs(x.x - y.x + 1.0f), abs(x.x - y.x - 1.0f)));
	}
	else if (/*$(Variable:sampleSpace)*/ == SampleSpace::Vector2)
	{
		K = -length(x.xy - y.xy);
	}
	else if (/*$(Variable:sampleSpace)*/ == SampleSpace::Vector3)
	{
		K = -length(x.xyz - y.xyz);
	}
	else if (/*$(Variable:sampleSpace)*/ == SampleSpace::Vector4)
	{
		K = -length(x - y);
	}
	else if (/*$(Variable:sampleSpace)*/ == SampleSpace::Sphere)
	{
		K = -acos(saturate(dot(2*x.xyz-1, 2*y.xyz-1)));
	}
	
	return K;
}

float combineFilter(int3 index, float filterX, float filterY, float filterZ)
{
	float F = 0.0f;
	if (/*$(Variable:separate)*/)
	{
		if (index.z == 0)
		{
			F += filterX * filterY* /*$(Variable:separateWeight)*/;
		}
		if (index.x == 0 && index.y == 0)
		{
			F += filterZ* (1.0f - /*$(Variable:separateWeight)*/);
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

/*$(_compute:Loss)*/(uint3 DTid : SV_DispatchThreadID)
{
	int3 index = DTid;
	float4 currentValue = SampleTexture[index];

	int3 otherIndex = getOtherIndex(index, /*$(Variable:key)*/, /*$(Variable:scrambleBits)*/);
	float4 otherValue = SampleTexture[otherIndex];

	uint3 textureSize = /*$(Variable:TextureSize)*/;

	int3 filterMin = /*$(Variable:filterMin)*/;
	int3 filterMax = /*$(Variable:filterMax)*/;
	int3 filterOffset = /*$(Variable:filterOffset)*/;

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
