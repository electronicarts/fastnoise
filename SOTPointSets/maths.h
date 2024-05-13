#pragma once

static const float c_pi = 3.14159265359f;
static const float c_goldenRatioConjugate = 0.61803398875f;  // 1 over phi, which is also just the fractional portion of phi

struct float2
{
	float x, y;
};

inline float Dot(const float2& a, const float2& b)
{
	return a.x * b.x + a.y * b.y;
}

inline float Length(const float2& f)
{
	return std::sqrt(Dot(f, f));
}

inline float DistanceSquared(const float2& a, const float2& b)
{
	float2 diff = float2{ a.x - b.x, a.y - b.y };
	return Dot(diff, diff);
}

// assumes the points are in [0,1]^2
inline float DistanceWrap(const float2& a, const float2& b)
{
	float2 diff;

	diff.x = std::abs(a.x - b.x);
	if (diff.x > 0.5f)
		diff.x = 1.0f - diff.x;

	diff.y = std::abs(a.y - b.y);
	if (diff.y > 0.5f)
		diff.y = 1.0f - diff.y;

	return Length(diff);
}

inline float2 Normalize(const float2& f)
{
	float len = Length(f);

	float2 ret;
	ret.x = f.x / len;
	ret.y = f.y / len;

	return ret;
}

inline float Lerp(float A, float B, float t)
{
	return A * (1.0f - t) + B * t;
}

template <typename T>
inline T Clamp(T value, T themin, T themax)
{
	if (value <= themin)
		return themin;

	if (value >= themax)
		return themax;

	return value;
}

inline float Fract(float f)
{
	return f - std::floor(f);
}
