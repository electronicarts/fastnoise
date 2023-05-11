# FastNoise

FastNoise generates noise textures optimized towards specific spatial and temporal filters, with specific per pixel data types.

This noise is intended to be used in stochastic rendering techniques at the lowest of sample counts to reduce error under spatial and temporal filters, or reduce the perceieved error when not filtered.

For information about how to use these noise textures, and deciding which options are right for your situation, please see [FastNoiseDesign](FastNoiseDesign.md)

## Building & Running

Building FastNoise.sln in visual studio 2022 will generate a FastNoise.exe file in the root folder of this repo.

Example command line:

`FastNoise.exe real uniform gauss 1.0 exponential 0.2 0.2 separate 0.5 128 128 64 out -split`

The above command generates 64 slices of noise which are each 128x128.  The noise is optimized
for a Gaussian low pass filter spatially, and separately, for exponential moving average temporally,
with an equal weighting to the spatial and temporal both filters. Each pixel stores a uniform random
scalar value.

## Command Line Parameters

Running the executable with no parameters will give the following output:

FastNoise.exe \<sampleSpace> \<distribution> \<filterXY> \<filterZ> \<filterCombine> \<textureSize> \<fileName> [-split]

  \<sampleSpace>  - The type of value stored in each pixel.
                     Real | Circle | Vector2 | Vector3 | Vector4 | Sphere

  \<distribution> - The distribution of values.
                     Uniform | Tent | Cosine | UniformHemisphere | Gauss

  \<filterXY>     - Spatial filter and parameters.
                     Box \<Size> |
                     Gauss \<Sigma> |
                     Binomial \<N>

  \<filterZ>      - Temporal filter and parameters.
                     Box \<Size> |
                     Gauss \<Sigma> |
                     Binomial \<N> |
                     Exponential \<Alpha> \<Beta>

  \<filterCombine> - How to combine the spatial and temporal filters.
                     Separate \<spatialWeight> |
                     Product |

  \<textureSize>   - The dimensions of the texture: x y z.

  \<fileName>      - The path and filename to output, without a file extension.

  -split          - If specified, each slice will be output as a separate image, else, all
                    slices will be put together into a single image.

  -numsteps \<steps> - specify how many iterations of optimization to do. defualts to 10,000.

Parameter Explanation:
- Box size is diameter, so 3 gives you 3x3, 5 gives you 5x5 etc.
- Binomial N is the N in N choose K, so 2 gives 3x3, 4 gives 5x5 etc.
- Exponential filter alpha is the value used in the lerp. See our paper for the meaning of beta.
- spatialWeight is how much to weigh the spatial filter. 1-spatialWeight is given to the temporal.


Note: Using gauss distribution will cause the output file to be a .hdr file to contain floating point
values. The values will be between 0 and 1, since .hdr files can't store negative numbers. To convert
these values to a sigma=1.0 gaussian distribution, subtract 0.5, then divide by 0.15.


Example:
  FastNoise.exe real uniform gauss 1.0 exponential 0.2 0.2 separate 0.5 128 128 64 out -split

The above command generates 64 slices of noise which are each 128x128.  The noise is optimized
for a Gaussian low pass filter spatially, and separately, for exponential moving average temporally,
with an equal weighting to the spatial and temporal both filters. Each pixel stores a uniform random
scalar value.

## How It Works

FastNoise.exe generates noise textures by initializing the noise volume to being stratified and then using simulated annealing to optimize the noise
towards a loss function.  It runs compute shaders using DX12 to perform this work on the GPU. See our paper for more details.

## Included Noise Textures

We've included some commonly used types of noise textures in the `out` folder but these are not the only types of noise possible.

 * `out/real/` - This contains scalar valued noise.  The temporal subfolder contains noise optimized for either gauss or exponential moving average (exp) over time and are meant to be used as flip books.
 * `out/sphere/` - This contains spherical and hemispherical valued noise.  The temporal subfolder again contains noise optimized to be used as a flip book over time.
