/*$(ShaderResources)*/

#include "fastnoise.hlsl"

/*$(_compute:Swap)*/(uint3 DTid : SV_DispatchThreadID)
{
	// 1. Total loss for the swap is sum of loss texture at source and destination
	uint3 index = DTid;
	uint3 otherIndex = getOtherIndex(index, /*$(Variable:key)*/, /*$(Variable:scrambleBits)*/);

	float loss = LossTexture[index] + LossTexture[otherIndex];

	// 2. Only one out of each pair does the swap, so check if we have the lower index
	int3 textureSize = /*$(Variable:TextureSize)*/;
	uint3 flatten = uint3(textureSize.y * textureSize.z, textureSize.z, 1);
	bool lesser = dot(flatten, index) < dot(flatten, otherIndex);

	// 3. Only do swap a fraction of the time, this helps convergence in the early iterations
	// TODO: We should adjust this based on the number of swaps - if it's above a threshold, then don't execute the swap


	uint iteration = /*$(Variable:Iteration)*/;
	uint randomSeed = wang_hash_init(DTid + iteration);
	uint randomValue = wang_hash_uint(randomSeed);
	uint swapSuppression = /*$(Variable:swapSuppression)*/;
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
