# FastNoise

FastNoise generates noise textures optimized towards specific spatial and temporal filters, with specific per pixel data types.

This noise is intended to be used in stochastic rendering techniques at the lowest of sample counts to reduce error under spatial and temporal filters, or reduce the perceieved error when not filtered.

For information about how to use these noise textures, and deciding which options are right for your situation, please see [FastNoise Design and Usage](FastNoiseDesign.md)

To see the various types of noises in frequency space, please see [Frequency Visualization of Common FAST Noise Types](DFTs.pdf)

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

FastNoise.exe \<sampleSpace> \<distribution> \<filterXY> \<filterZ> \<filterCombine> \<textureSize> \<fileName> [Options]

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

Options:

  -split             - If specified, each slice will be output as a separate image, else, all
                       slices will be put together into a single image.

  -numsteps \<steps> - specify how many iterations of optimization to do. defualts to 10,000.

  -output \<type>    - Force an output type.  type can be: exr, csv.

  -seed \<value>     - Force the random seed value. Makes process deterministic.

  -init \<filename>  - Load data for initial state instead of init.hlsl generating it. Binary
                       file must contain textureSize.x * textureSize.y * textureSize.z * 4 floats.

  -progress \<count>  - Shows this many progress images before the end. Defaults to 0.

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

We've included some commonly used types of noise textures in the noise.zip file but these are not the only types of noise possible.

The first folder names describe the type of value in each pixel: gaussian (scalar), real (scalar), sphere, vector2 or vector 3.

Within those folders are purely spatial noise, with various distributions of values (E.g. uniform vs cosine hemisphere weighted).

Subfolders contain temporal noise that is either optimized for EMA over time (exp) or for a gaussian filter over time (gauss).

Product noise multiplies the spatial and temporal filters together. Separate noise adds them.

Please see [FastNoise Design and Usage](FastNoiseDesign.md) for more information about what type of noise to use under specific circumstances.

## Resources

[ea.com: Filter-Adapted Spatio-Temporal Sampling for Real-Time Rendering](https://www.ea.com/seed/news/spatio-temporal-sampling)

[Arxiv: Filter-adapted spatiotemporal sampling for real-time rendering](https://arxiv.org/abs/2310.15364)

Our work will appear at i3D 20204. Links will be added when available!

## Authors

<p align="center"><a href="https://seed.ea.com"><img src="logo/SEED.jpg" width="150px"></a><br>
<b>Search for Extraordinary Experiences Division (SEED) - Electronic Arts <br> http://seed.ea.com</b><br>
We are a cross-disciplinary team within EA Worldwide Studios.<br>
Our mission is to explore, build and help define the future of interactive entertainment.</p>

Code and paper by William Donnelly, Alan Wolfe, Judith Bütepage and Jon Valdés.

## Contributing

Before you can contribute, EA must have a Contributor License Agreement (CLA) on file that has been signed by each contributor.
You can sign here: http://bit.ly/electronic-arts-cla
