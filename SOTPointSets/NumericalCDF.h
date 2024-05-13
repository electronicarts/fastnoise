#pragma once

#include "maths.h"
#include <vector>
#include <algorithm>

#include "stb/stb_image.h"

struct CDF
{
	std::vector<float2> CDFSamples;

	float InverseCDF(float y) const
	{
		auto it = std::lower_bound(CDFSamples.begin(), CDFSamples.end(), y,
			[] (const float2& CDFSample, float y)
			{
				return CDFSample.y < y;
			}
		);

		// If the first item is greater than y, return 0
		if (it == CDFSamples.begin())
		{
			return CDFSamples[0].y;
		}
		// If the last item is smaller than y, return 1
		else if (it == CDFSamples.end())
		{
			return CDFSamples[CDFSamples.size() - 1].y;
		}
		// otherwise, interpolate from it-1 to it
		else
		{
			// The index of the value lower than the value we are searching for
			size_t index = it - CDFSamples.begin() - 1;

			// Get the percent we are between the y values
			float percentY = (y - CDFSamples[index].y) / (CDFSamples[index + 1].y - CDFSamples[index].y);

			// Use that percent to go from the previous to current percent
			return Lerp(CDFSamples[index].x, CDFSamples[index + 1].x, percentY);
		}
	}
};

template <typename TCDFLambda>
CDF CDFFromCDFFn(float minX, float maxX, int numSamples, const TCDFLambda& CDFLambda)
{
	CDF ret;

	// Make CDF samples
	ret.CDFSamples.resize(numSamples);
	for (int i = 0; i < numSamples; ++i)
	{
		// We calculate percent this way so we hit 0% and 100% both.
		// Normally you'd want to "keep away from the edges" but we want the full range of data here
		float percent = float(i) / float(numSamples - 1);

		float x = Lerp(minX, maxX, percent);
		float y = CDFLambda(x);
		ret.CDFSamples[i] = float2{ x, y };
	}

	return ret;
}

// pixel values are flipped so that 0 (darker) are denser, and 1 (lighter) are less dense.
struct DensityMap
{
	int w = 0;
	int h = 0;
	std::vector<float> pixels;
};

inline DensityMap LoadDensityMap(const char* fileName)
{
	DensityMap ret;

	// Load the image if we can
	int c;
	unsigned char* pixels = stbi_load(fileName, &ret.w, &ret.h, &c, 1);
	if (!pixels)
	{
		printf("[ERROR] Could not load image file %s!\n", fileName);
		return ret;
	}

	// convert to float and store it
	ret.pixels.resize(ret.w * ret.h);
	for (int i = 0; i < ret.w * ret.h; ++i)
		ret.pixels[i] = 1.0f - float(pixels[i]) / 255.0f;

	// Free the loaded image
	stbi_image_free(pixels);

	return ret;
}

inline CDF CDFFromDensityMap(const DensityMap& densityMap, int numSamples, float2 projectionDirection)
{
	CDF ret;

	float maxX = (std::abs(projectionDirection.x) + std::abs(projectionDirection.y)) * 0.5f;
	float minX = -maxX;

	// Make initial CDF samples, but with a y value of 0. we are going to add the projected pixel values in and then normalize it
	ret.CDFSamples.resize(numSamples);
	for (int i = 0; i < numSamples; ++i)
	{
		// We calculate percent this way so we hit 0% and 100% both.
		// Normally you'd want to "keep away from the edges" but we want the full range of data here
		float percent = float(i) / float(numSamples - 1);
		float x = Lerp(minX, maxX, percent);
		ret.CDFSamples[i] = float2{ x, 0.0f};
	}

	// Project the pixel values and sum them up to make a PDF
	for (int iy = 0; iy < densityMap.h; ++iy)
	{
		float normalizedIyMin = float(iy) / float(densityMap.h) - 0.5f;
		float normalizedIyMax = float(iy + 1) / float(densityMap.h) - 0.5f;

		for (int ix = 0; ix < densityMap.w; ++ix)
		{
			float pixelValue = densityMap.pixels[iy * densityMap.w + ix];

			float normalizedIxMin = float(ix) / float(densityMap.w) - 0.5f;
			float normalizedIxMax = float(ix + 1) / float(densityMap.w) - 0.5f;

			float projMin = Dot(float2{ normalizedIxMin, normalizedIyMin }, projectionDirection);
			float projMax = Dot(float2{ normalizedIxMax, normalizedIyMax }, projectionDirection);

			// assumes a square of +/- 0.5
			float maxX = (std::abs(projectionDirection.x) + std::abs(projectionDirection.y)) * 0.5f;
			float minX = -maxX;
			projMin = (projMin - minX) / (maxX - minX);
			projMax = (projMax - minX) / (maxX - minX);

			if (projMin > projMax)
				std::swap(projMin, projMax);

			float bucketMin = Clamp(projMin * float(numSamples), 0.0f, float(numSamples));
			float bucketMax = Clamp(projMax * float(numSamples), 0.0f, float(numSamples));

			while (bucketMin < bucketMax)
			{
				int bucketIndex = int(std::floor(bucketMin));
				if (bucketIndex >= numSamples)
					break;

				float endOfBucket = std::min(std::floor(bucketMin + 1), bucketMax);
				float bucketPercent = endOfBucket - bucketMin;

				ret.CDFSamples[bucketIndex].y += pixelValue * bucketPercent;

				bucketMin = float(bucketIndex + 1);
			}
		}
	}

	// Convert the PDF to CDF by making every sample by the sum of the samples that are at or before it
	for (int i = 1; i < numSamples; ++i)
		ret.CDFSamples[i].y += ret.CDFSamples[i - 1].y;

	// Normalize the CDF so that the first entry is 0, and the last entry is 1
	float minCDFSample = ret.CDFSamples[0].y;
	float maxCDFSample = ret.CDFSamples[numSamples - 1].y;
		
	for (float2& p : ret.CDFSamples)
		p.y = (p.y - minCDFSample) / (maxCDFSample - minCDFSample);


	return ret;
}
