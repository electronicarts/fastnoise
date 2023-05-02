#///////////////////////////////////////////////////////////////////////////////
#//               FastNoise - F.A.S.T. Sampling Implementation                //
#//         Copyright (c) 2023 Electronic Arts Inc. All rights reserved.      //
#///////////////////////////////////////////////////////////////////////////////

import os.path

computeHistogram = True
computeSpectrum = True

width=128
height=128

# Generate sample textures, together with histograms and noise spectrum
for (space, distribution) in [("real", "uniform"), ("real", "tent"), ("circle", "uniform"), ("vector2", "uniform"), ("sphere", "uniform"), ("sphere", "cosine"), ("vector3", "uniform"), ("vector4", "uniform")]:
    for (filter, param) in [("box", 3), ("box", 5), ("binomial", 2), ("binomial", 3), ("gauss", 0.7), ("gauss", 1.0)]:

        os.makedirs(f"analysis/{space}", exist_ok = True)
        filename = f"analysis/{space}/{space}_{distribution}_{filter}_{param}"

        if not os.path.isfile(filename + ".png"):
            cmd = f"FastNoise.exe {space} {distribution} {filter} {param} box 1 product {width} {height} 1 {filename}"
            print(cmd)
            os.system(cmd)

        if computeHistogram and not os.path.isfile(filename + "_histogram.png"):
            cmd = f"python scripts/histogram.py {filename}.png {space}"
            print(cmd)
            os.system(cmd)
            
        if computeSpectrum and not os.path.isfile(filename + "_spectrum.png"):
            cmd = f"python scripts/spectrum.py {filename}.png {space}"
            print(cmd)
            os.system(cmd)