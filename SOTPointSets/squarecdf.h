#pragma once

#include "maths.h"

// Derived By William Donnelly

// The square is centered at (0,0) and has width and height of 1.
// So, the square is (x,y) in (-0.5,0.5)^2
// The CDF / ICDF are then shiften to be between -0.5 and 0.5, instead of being from 0 to 1.
namespace Square
{
	inline float InverseCDF(float u, float2 d)
	{
		float c = std::max(std::abs(d.x), std::abs(d.y));
		float s = std::min(std::abs(d.x), std::abs(d.y));

		if (2 * c * std::abs(u) < c - s)
		{
			return c * u;
		}
		else
		{
			float t = 0.5f * (c + s) - sqrtf(2.0f * s * c * (0.5f - std::abs(u)));
			return (u < 0) ? -t : t;
		}
	}

	inline float CDF(float x, float2 d)
	{
		float c = std::max(std::abs(d.x), std::abs(d.y));
		float s = std::min(std::abs(d.x), std::abs(d.y));

		if (std::abs(x) > 0.5 * (c + s))
		{
			return (x < 0.0f) ? -0.5f : 0.5f;
		}
		else if (std::abs(x) < 0.5f * (c - s))
		{
			return x / c;
		}
		else
		{
			float fromEnd = (0.5f * (c + s) - std::abs(x));
			float u = 0.5f - 0.5f * fromEnd * fromEnd / (c * s);
			return (x < 0.0f) ? -u : u;
		}
	}

	inline float PDF(float x, float2 d)
	{
		float c = std::max(std::abs(d.x), std::abs(d.y));
		float s = std::min(std::abs(d.x), std::abs(d.y));

		if (abs(x) < 0.5f * (c - s))
		{
			return 1 / c;
		}
		else if (abs(x) < 0.5f * (c + s))
		{
			float fromEnd = (0.5f * (c + s) - std::abs(x));
			return fromEnd / (c * s);
		}
		else
		{
			return 0.0f;
		}
	}
};