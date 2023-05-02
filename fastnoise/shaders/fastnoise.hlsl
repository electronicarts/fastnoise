///////////////////////////////////////////////////////////////////////////////
//               FastNoise - F.A.S.T. Sampling Implementation                //
//         Copyright (c) 2023 Electronic Arts Inc. All rights reserved.      //
///////////////////////////////////////////////////////////////////////////////

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

uint roundFunction(uint subkey, uint r)
{
	uint seed = subkey ^ r;
	return wang_hash_uint(seed);
}

// Permutes xy components of the index based on a random "key"
// bits is the number of bits to scramble
uint3 getOtherIndex(uint3 index, uint4 key, uint bits)
{
	uint mask = (1 << bits) - 1;
	uint2 lr = index.xy & mask;
	uint2 highIndex = index.xy & ~mask;

	// Use other bits as part of randomization
	key ^= index.zzzz ^ highIndex.xyxy;

	// 3 round Feistel network
	lr = lr.yx ^ uint2(0, roundFunction(key.x, lr.y) & mask);
	lr = lr.yx ^ uint2(0, roundFunction(key.y, lr.y) & mask);
	lr = lr.yx ^ uint2(0, roundFunction(key.z, lr.y) & mask);

	// XOR with the final component of the key
	lr ^= uint2(key.w >> bits, key.w) & mask;

	// 3 round Feistel network - inverse
	lr = lr.yx ^ uint2(roundFunction(key.z, lr.x) & mask, 0);
	lr = lr.yx ^ uint2(roundFunction(key.y, lr.x) & mask, 0);
	lr = lr.yx ^ uint2(roundFunction(key.x, lr.x) & mask, 0);

	return uint3(highIndex | lr, index.z);
}


float2 squareToDiskPolar(float2 u)
{
	float2 rTheta;
	if (u.x == 0 && u.y == 0)
		rTheta = float2(0, 0);

	float r, theta;
	if (abs(u.x) > abs(u.y))
	{
		rTheta.x = u.x;
		rTheta.y = 0.78539816339f * (u.y / u.x);
	}
	else
	{
		rTheta.x = u.y;
		rTheta.y = 1.57079632679f - 0.78539816339f * (u.x / u.y);
	}
	return rTheta;
}

// Split a single uint into pieces with different numbers of bits in each component
uint2 splitBits(uint x, uint2 bits)
{
	return (x >> uint2(bits.y, 0)) & ((1 << bits) - 1);
}

uint3 splitBits(uint x, uint3 bits)
{
	return (x >> uint3(bits.y + bits.z, bits.z, 0)) & ((1 << bits) - 1);
}

uint4 splitBits(uint x, uint4 bits)
{
	return (x >> uint4(bits.y + bits.z + bits.w, bits.z + bits.w, bits.w, 0)) & ((1 << bits) - 1);
}



// Scramble low 4 bits of both components of x using multiplication by a
// TODO: generalize to 3D and variable number of low bits
uint2 scramble(uint2 x, uint a)
{
	 //Put low bits of x into a single number
	uint n = ((x.x & 0xF) << 4) | (x.y & 0xF);

	// Multiply by a mod 2^8
	uint m = (n * a) & 0xFF;

	// Keep high bits of x, but use lower bits of m
	uint2 y = x & uint2(0xFFFFFFF0, 0xFFFFFFF0);
	y.x |= (m >> 4) & 0xF;
	y.y |= (m & 0xF);
	return y;
}
