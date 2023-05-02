#///////////////////////////////////////////////////////////////////////////////
#//               FastNoise - F.A.S.T. Sampling Implementation                //
#//         Copyright (c) 2023 Electronic Arts Inc. All rights reserved.      //
#///////////////////////////////////////////////////////////////////////////////

import os.path

computeHistogram = True
computeSpectrum = True

width=128
height=128
depth=64

# Temporal comparison with STBN
for (space,distribution) in [("real", "uniform")]:
    for (filter,param) in [("gauss", 1.3435)]:

        # Filter alpha is what we will use to test the noise. Does not have to be the same as the generated noise.
        for filterAlpha in [0.1]:

            directory = f"analysis/temporal/{space}-{width}x{height}x{depth}"
            os.makedirs(directory, exist_ok = True)
            
            # Compare with STBN and white noise
            csvfiles = [f"{directory}/{space}_stbn_temporal.csv", 
                        f"{directory}/{space}_whitenoise_temporal.csv",
                        f"{directory}/{space}_{distribution}_{filter}_{param}_white_over_time_temporal.csv"]

            for alpha in [0.1]:

                for beta in [0.1, 0.2, 0.3, 0.4, 0.5]:
        
                    #for filterCombine in ["product", "separate 0.5"]:
                    for filterCombine in ["product"]:

                        filename = f"{directory}/{space}_{distribution}_{filter}_{param}_exponential_{alpha}_{beta}_{filterCombine.replace(' ', '_')}"
                
                        if not os.path.isfile(filename + ".png"):

                            cmd = f"FastNoise.exe {space} {distribution} {filter} {param} exponential {alpha} {beta} {filterCombine} {width} {height} {depth} {filename}"

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

                        if not os.path.isfile(filename + "_temporal.csv"):
                            cmd = f"python scripts/temporal.py {filename}.png {space} {filter} {param} {filterAlpha}"
                            print(cmd)
                            os.system(cmd)                

                        csvfiles.append(filename + "_temporal.csv")
            
            # plot generated CSV files for the same space together
            cmd = f"python scripts/temporal-plot.py {directory}/{space}_{distribution}_{filter}_{param}_exponential_{alpha}_temporal.png " + " ".join(csvfiles)
            print(cmd)
            os.system(cmd)