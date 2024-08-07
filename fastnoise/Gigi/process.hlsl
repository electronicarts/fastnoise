/*$(ShaderResources)*/

uint wang_hash_init(uint3 seed)
{
	return uint(seed.x * uint(1973) + seed.y * uint(9277) + seed.z * uint(26699)) | uint(1);
}

uint wang_hash_uint(inout uint seed)
{
	seed = uint(seed ^ uint(61)) ^ uint(seed >> uint(16));
	seed *= uint(9);
	seed = seed ^ (seed >> 4);
	seed *= uint(0x27d4eb2d);
	seed = seed ^ (seed >> 15);
	return seed;
}

float wang_hash_float01(inout uint state)
{
	return float(wang_hash_uint(state) & 0x00FFFFFF) / float(0x01000000);
}

/*$(_compute:Process)*/(uint3 DTid : SV_DispatchThreadID)
{
    float4 input = Texture[DTid.xy];

    // use the iteration index as the per pixel rng seed to get different random numbers each iteration
    uint rng = wang_hash_init(uint3(DTid.xy, /*$(Variable:Iteration)*/));

    // as a test, monte carlo integrate some noise
    Texture[DTid.xy] = lerp(input.rgba, float4(wang_hash_float01(rng), wang_hash_float01(rng), 0.0f, 1.0f), 1.0f / float(1+/*$(Variable:Iteration)*/));

    // Sum up the iteration values. Do this so we can see the value on the CPU to see how to shuttle data back from GPU to CPU
    if (DTid.x == 0 && DTid.y == 0)
        Data[0].iterationSum = Data[0].iterationSum + /*$(Variable:Iteration)*/;
}
