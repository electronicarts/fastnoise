#define _CRT_SECURE_NO_WARNINGS // for stb

// Settings
static const float c_imageGaussBlobSigma = 1.5f;

#include <random>
#include <vector>
#include <direct.h>
#include <stdio.h>
#include <chrono>
#include <string>
#include <filesystem>

#include "squarecdf.h"
#include "NumericalCDF.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#define MULTITHREADED() true

unsigned int g_seed = 0;

// Settings for this execution
struct Settings
{
	std::string maskFileName;
	unsigned int pointCount = 0;
	unsigned int setCount = 0;
	unsigned int outputSize = 0;
	unsigned int batchCount = 0;
	unsigned int batchSize = 0;
};
Settings g_settings;

std::mt19937 GetRNG(int index)
{
	std::mt19937 ret(g_seed + index);
	return ret;
}

template <int NumChannels>
void PlotGaussian(std::vector<unsigned char>& image, int width, int height, int x, int y, float sigma, unsigned char color[NumChannels])
{
	int kernelRadius = int(std::sqrt(-2.0f * sigma * sigma * std::log(0.005f)));

	int sx = Clamp(x - kernelRadius, 0, width - 1);
	int ex = Clamp(x + kernelRadius, 0, height - 1);
	int sy = Clamp(y - kernelRadius, 0, width - 1);
	int ey = Clamp(y + kernelRadius, 0, height - 1);

	for (int iy = sy; iy <= ey; ++iy)
	{
		unsigned char* pixel = &image[(iy * width + sx) * NumChannels];

		int ky = std::abs(iy - y);
		float kernelY = std::exp(-float(ky * ky) / (2.0f * sigma * sigma));

		for (int ix = sx; ix <= ex; ++ix)
		{
			int kx = std::abs(ix - x);
			float kernelX = std::exp(-float(kx * kx) / (2.0f * sigma * sigma));

			float kernel = kernelX * kernelY;

			for (int i = 0; i < NumChannels; ++i)
			{
				unsigned char oldColor = *pixel;
				unsigned char newColor = (unsigned char)Lerp(float(oldColor), float(color[i]), kernel);
				*pixel = newColor;
				pixel++;
			}
		}
	}
}

void SavePointSet(const std::vector<float2>& points, const char* baseFileName, int setIndex)
{
	/*
	// find min/max value of points and print it out
	{
		float minx = points[0].x;
		float maxx = points[0].x;
		float miny = points[0].y;
		float maxy = points[0].y;

		for (size_t index = 0; index < points.size(); ++index)
		{
			minx = std::min(minx, points[index].x);
			maxx = std::max(maxx, points[index].x);
			miny = std::min(miny, points[index].y);
			maxy = std::max(maxy, points[index].y);
		}

		printf("(%f, %f) - (%f, %f)\n", minx, miny, maxx, maxy);
	}
	*/

	// Write out points in text
	{
		char fileName[1024];
		sprintf_s(fileName, "%s.%i.txt", baseFileName, setIndex);
		FILE* file = nullptr;
		fopen_s(&file, fileName, "wb");

		fprintf(file, "float points[%i][2] =\n{\n", (int)points.size());
		
		for (size_t index = 0; index < points.size(); ++index)
			fprintf(file, "    { %ff, %ff },\n", Clamp(points[index].x * 0.5f + 0.5f, 0.0f, 1.0f), Clamp(points[index].y * 0.5f + 0.5f, 0.0f, 1.0f));

		fprintf(file, "};\n");
		
		fclose(file);
	}

	// Write out points in binary
	{
		char fileName[1024];
		sprintf_s(fileName, "%s.dat", baseFileName);
		FILE* file = nullptr;
		fopen_s(&file, fileName, (setIndex == 0 ? "wb" : "ab"));

		float out[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		for (size_t index = 0; index < points.size(); ++index)
		{
			out[0] = Clamp(points[index].x * 0.5f + 0.5f, 0.0f, 1.0f);
			out[1] = Clamp(points[index].y * 0.5f + 0.5f, 0.0f, 1.0f);
			fwrite(out, sizeof(float), 4, file);
		}

		fclose(file);
	}

	// Draw an image of the points
	{
		std::vector<unsigned char> pixels(g_settings.outputSize * g_settings.outputSize, 255);
		std::vector<unsigned char> pixelsGauss(g_settings.outputSize * g_settings.outputSize, 255);

		for (size_t index = 0; index < points.size(); ++index)
		{
			int x = (int)Clamp((points[index].x * 0.5f + 0.5f) * float(g_settings.outputSize - 1), 0.0f, float(g_settings.outputSize - 1));
			int y = (int)Clamp((points[index].y * 0.5f + 0.5f) * float(g_settings.outputSize - 1), 0.0f, float(g_settings.outputSize - 1));
			pixels[y * g_settings.outputSize + x] = 0;

			unsigned char color[] = { 0 };
			x = (int)Clamp((points[index].x * 0.5f + 0.5f) * float(g_settings.outputSize - 1), 0.0f, float(g_settings.outputSize - 1));
			y = (int)Clamp((points[index].y * 0.5f + 0.5f) * float(g_settings.outputSize - 1), 0.0f, float(g_settings.outputSize - 1));
			PlotGaussian<1>(pixelsGauss, g_settings.outputSize, g_settings.outputSize, x, y, c_imageGaussBlobSigma, color);
		}

		char fileName[1024];
		sprintf_s(fileName, "%s.%i.pixel.png", baseFileName, setIndex);
		stbi_write_png(fileName, g_settings.outputSize, g_settings.outputSize, 1, pixels.data(), 0);

		sprintf_s(fileName, "%s.%i.gauss.png", baseFileName, setIndex);
		stbi_write_png(fileName, g_settings.outputSize, g_settings.outputSize, 1, pixelsGauss.data(), 0);
	}
}

float2 MakeDirection_Gauss(int iterationIndex, int batchIndex, int batchSize)
{
	std::mt19937 rng = GetRNG(iterationIndex * batchSize + batchIndex);
	std::normal_distribution<float> distNormal(0.0f, 1.0f);

	// Make a uniform random unit vector by generating 2 normal distributed values and normalizing the result.
	float2 direction;
	direction.x = distNormal(rng);
	direction.y = distNormal(rng);
	return Normalize(direction);
}

float2 MakeDirection_GoldenRatio(int iterationIndex, int batchIndex, int batchSize)
{
	std::mt19937 rng = GetRNG(batchIndex);
	std::uniform_real_distribution<float> distUniform(0.0f, 1.0f);

	float value01 = distUniform(rng);
	for (int i = 0; i < iterationIndex; ++i)
		value01 = Fract(value01 + c_goldenRatioConjugate);

	float angle = value01 * 2.0f * c_pi;

	return float2
	{
		std::cos(angle),
		std::sin(angle)
	};
}

template <typename TMakeDirectionLambda, typename TBatchBeginLambda, typename TBatchEndLambda, typename TICDFLambda>
void GeneratePoints(int numPoints, int numIterations, int batchSize, const char* baseFileName, bool stratifyLine, int setIndex, const TMakeDirectionLambda& MakeDirectionLambda, const TBatchBeginLambda& BatchBeginLambda, const TBatchEndLambda& BatchEndLambda, const TICDFLambda& ICDFLambda)
{
	// get the timestamp of when this started
	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

	printf("%s.%i\n", baseFileName, setIndex);

	FILE* file = nullptr;
	char outputFileNameCSV[1024];
	sprintf(outputFileNameCSV, "%s.%i.csv", baseFileName, setIndex);
	fopen_s(&file, outputFileNameCSV, "wb");
	fprintf(file, "\"Iteration\",\"Avg. Movement\"\n");

	// Generate the starting points
	std::vector<float2> points(numPoints);
	{
		std::mt19937 rng = GetRNG(0);
		std::uniform_real_distribution<float> distUniform(-1.0f, 1.0f);
		for (float2& p : points)
		{
			p.x = distUniform(rng);
			p.y = distUniform(rng);
		}
	}

	// Per batch data
	// Each batch entry has it's own data so the batches can be parallelized
	struct BatchData
	{
		BatchData(int numPoints)
		{
			sorted.resize(numPoints);
			for (int i = 0; i < numPoints; ++i)
				sorted[i] = i;
			projections.resize(numPoints);
			batchDirections.resize(numPoints);
		}

		std::vector<int> sorted;
		std::vector<float> projections;
		std::vector<float2> batchDirections;
	};
	std::vector<BatchData> allBatchData(batchSize, BatchData(numPoints));

	// For each iteration
	int lastPercent = -1;
	for (int iterationIndex = 0; iterationIndex < numIterations; ++iterationIndex)
	{
		// Do the batches in parallel
		#if MULTITHREADED()
		#pragma omp parallel for
		#endif
		for (int batchIndex = 0; batchIndex < batchSize; ++batchIndex)
		{
			BatchData& batchData = allBatchData[batchIndex];

			float2 direction = MakeDirectionLambda(iterationIndex, batchIndex, batchSize);

			// project the points
			for (size_t i = 0; i < numPoints; ++i)
				batchData.projections[i] = Dot(direction, points[i]);

			// sort the projections
			std::sort(batchData.sorted.begin(), batchData.sorted.end(),
				[&](uint32_t a, uint32_t b)
				{
					return batchData.projections[a] < batchData.projections[b];
				}
			);

			// update batchDirections
			std::mt19937 rng = GetRNG(iterationIndex * batchSize + batchIndex);
			std::uniform_real_distribution<float> distJitter(0.0f, 1.0f);
			void* param = BatchBeginLambda(direction);
			for (size_t i = 0; i < numPoints; ++i)
			{
				float jitter = 0.5f;
				if (stratifyLine)
					jitter = distJitter(rng);

				float targetProjection = ((float(i) + jitter) / float(numPoints));

				targetProjection = ICDFLambda(param, targetProjection, direction);

				float projDiff = targetProjection - batchData.projections[batchData.sorted[i]];

				batchData.batchDirections[batchData.sorted[i]].x = direction.x * projDiff;
				batchData.batchDirections[batchData.sorted[i]].y = direction.y * projDiff;
			}
			BatchEndLambda(param);
		}

		// average all batch directions into batchDirections[0]
		{
			for (int batchIndex = 1; batchIndex < batchSize; ++batchIndex)
			{
				float alpha = 1.0f / float(batchIndex + 1);
				for (size_t i = 0; i < numPoints; ++i)
				{
					allBatchData[0].batchDirections[i].x = Lerp(allBatchData[0].batchDirections[i].x, allBatchData[batchIndex].batchDirections[i].x, alpha);
					allBatchData[0].batchDirections[i].y = Lerp(allBatchData[0].batchDirections[i].y, allBatchData[batchIndex].batchDirections[i].y, alpha);
				}
			}
		}

		// update points
		float totalDistance = 0.0f;
		for (size_t i = 0; i < numPoints; ++i)
		{
			const float2& adjust = allBatchData[0].batchDirections[i];

			points[i].x += adjust.x;
			points[i].y += adjust.y;

			totalDistance += std::sqrt(adjust.x * adjust.x + adjust.y * adjust.y);
		}

		int percent = int(100.0f * float(iterationIndex) / float(numIterations - 1));
		if (percent != lastPercent)
		{
			lastPercent = percent;
			printf("\r[%i%%] %f", percent, totalDistance / float(numPoints));
			fprintf(file, "\"%i\",\"%f\"\n", iterationIndex, totalDistance / float(numPoints));
		}
	}
	printf("\n");

	fclose(file);

	// Write out the final results
	SavePointSet(points, baseFileName, setIndex);

	// report how long this took
	float elpasedSeconds = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() - start).count();
	printf("%0.2f seconds\n\n", elpasedSeconds);
}

bool GetFromString(std::string& value, const char* s)
{
	value = s;
	return true;
}

bool GetFromString(unsigned int& value, const char* s)
{
	return sscanf(s, "%u", &value) == 1;
}

template <typename T>
void GetFromCommandLine(int argc, char** argv, int& argIndex, bool& commandLineOK, T& value, const char* label)
{
	if (!commandLineOK)
		return;

	if (argIndex >= argc || !GetFromString(value, argv[argIndex]))
	{
		printf("Error: Could not read positional argument \"%s\"\n", label);
		commandLineOK = false;
		return;
	}
	argIndex++;
}

template <typename T>
bool GetFromCommandLineOptional(int argc, char** argv, int& argIndex, bool& commandLineOK, T& value, const char* label)
{
	if (!commandLineOK || argIndex >= argc)
		return false;

	if (_stricmp(argv[argIndex], label))
		return false;
	argIndex++;

	if (argIndex >= argc || !GetFromString(value, argv[argIndex]))
	{
		printf("Error: Could not read argument for option \"%s\"\n", label);
		commandLineOK = false;
		return true;
	}
	argIndex++;

	return true;
}

int main(int argc, char** argv)
{
	// set a random seed.  It can be overridden by command line parameter
	{
		std::random_device rd;
		g_seed = rd();
	}

	// Read the command line in
	{
		bool commandLineOK = true;
		int argIndex = 1;

		GetFromCommandLine(argc, argv, argIndex, commandLineOK, g_settings.maskFileName, "fileName");
		GetFromCommandLine(argc, argv, argIndex, commandLineOK, g_settings.pointCount, "pointCount");
		GetFromCommandLine(argc, argv, argIndex, commandLineOK, g_settings.setCount, "setCount");
		GetFromCommandLine(argc, argv, argIndex, commandLineOK, g_settings.batchCount, "batchCount");
		GetFromCommandLine(argc, argv, argIndex, commandLineOK, g_settings.batchSize, "batchSize");
		GetFromCommandLine(argc, argv, argIndex, commandLineOK, g_settings.outputSize, "outputSize");

		while (commandLineOK && argIndex < argc)
		{
			bool advanced = false;
			advanced |= GetFromCommandLineOptional(argc, argv, argIndex, commandLineOK, g_seed, "-seed");

			if (!advanced)
			{
				printf("[Error] Unknown command line option: \"%s\"\n", argv[argIndex]);
				commandLineOK = false;
			}
		}

		if (!commandLineOK)
		{
			printf(
				"\n"
				"Usage: SOTPointSets <fileName> <pointCount> <setCount> <batchCount> <batchSize> <outputSize> [Options]\n"
				"\n"
				"This program generates points in a square, using an image as a probability mask for\n"
				"where the points should be placed, using sliced optimal transport.\n"
				"\n"
				"Parameters:\n"
				"\n"
				"<filename>   - An image file to use as a mask for 2d point generation.\n"
				"<pointcount> - How many points to generate per set.\n"
				"<setcount>   - How many sets to make.\n"
				"<batchcount> - How many batches for each set. (try 1000)\n"
				"<batchsize>  - How many adjustments per batch. (try 64)\n"
				"<outputsize> - The size (width and height) of the output image showing the points.\n"
				"\n"
				"Options:\n"
				"\n"
				"-seed        - Specify the random seed. Useful for making this deterministic.\n"
				"\n"
				"Output Files:\n"
				"\n"
				".csv         - Shows you the convergence rate of the point set. Useful for tuning batches.\n"
				".png         - Shows you the point set on a square.\n"
				".txt         - a copy/pastable C++ array of the point sets. Points in [0,1]^2.\n"
				".dat         - a binary file of ALL point sets as float4s, with z=0 and w=1. Points in [0,1]^2.\n"
			);
			return 1;
		}
	}

	// report what we are doing
	printf(
		"Loading:     %s\n"
		"point count: %u\n"
		"set count:   %u\n"
		"batch count: %u\n"
		"batch size:  %u\n"
		"output size: %u\n"
		"seed:        %u\n"
		"\n",
		g_settings.maskFileName.c_str(),
		g_settings.pointCount,
		g_settings.setCount,
		g_settings.batchCount,
		g_settings.batchSize,
		g_settings.outputSize,
		g_seed
	);

	_mkdir("out");

	// Points in square, with a density map
	{
		DensityMap densityMap = LoadDensityMap(g_settings.maskFileName.c_str());

		std::filesystem::path baseFileNameOut = std::filesystem::path("out") / std::filesystem::path(g_settings.maskFileName).stem();

		for (unsigned int setIndex = 0; setIndex < g_settings.setCount; ++setIndex)
		{
			GeneratePoints(g_settings.pointCount, g_settings.batchCount, g_settings.batchSize, baseFileNameOut.string().c_str(), false, setIndex, MakeDirection_Gauss,
				// Batch Begin
				[&](const float2& direction)
				{
					// Make ICDF by projecting density map onto the direction
					CDF* ret = new CDF;
					*ret = CDFFromDensityMap(densityMap, 1000, direction);
					for (float2& p : ret->CDFSamples)
						p.y -= 0.5f;
					return ret;
				},
				// Batch End
				[](void* param)
				{
					CDF* cdf = (CDF*)param;
					delete cdf;
				},
				// ICDF
				[](void* param, float y, const float2& direction)
				{
					// Convert y: square is in [-0.5, 0.5], but y is in [0, 1].
					y = y - 0.5f;

					// Evaluate ICDF
					float x = ((CDF*)param)->InverseCDF(y);


					// The CDF is in [-0.5, 0.5], but we want the points to be in [-1,1]
					return x * 2.0f;
				}
			);

			// Deterministically make a new seed for the next run
			g_seed = (unsigned int)std::hash<unsigned int>()(g_seed);
		}
	}

	return 0;
}
