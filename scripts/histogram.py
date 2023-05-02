#///////////////////////////////////////////////////////////////////////////////
#//               FastNoise - F.A.S.T. Sampling Implementation                //
#//         Copyright (c) 2023 Electronic Arts Inc. All rights reserved.      //
#///////////////////////////////////////////////////////////////////////////////

import sys

import matplotlib.pyplot as plt
import numpy as np
from matplotlib import image
import imageio

# imageio is for hdr support.
# https://matiascodesal.com/blog/how-read-hdr-image-using-python/

if len(sys.argv) != 3:
    print(f"Usage: {sys.argv[0]} filename sampleSpace")
    print("Where sampleSpace is (real|circle|sphere|vector2|vector3|vector4)")
    exit(1)

img = None

isHDR = sys.argv[1].endswith(".hdr")

if isHDR:
    imageio.plugins.freeimage.download()
    img = imageio.v2.imread(sys.argv[1], format='HDR-FI')
    sys.argv[1] = sys.argv[1].replace(".hdr", ".png")
else:
    img = image.imread(sys.argv[1])


if sys.argv[2] == "circle" or sys.argv[2] == "real":

    img = img[:,:,0]
    plt.clf()
    plt.hist(img.flatten(), bins=256, range=(0,1))
    plt.savefig(sys.argv[1].replace(".png", "_histogram.png"))
elif sys.argv[2] == "vector2":

    fig, axs = plt.subplots(1,3, figsize=(15, 5))

    axs[0].hist(img[:,:,0].flatten(), bins=256, range=(0,1))
    axs[1].hist(img[:,:,1].flatten(), bins=256, range=(0,1))
    axs[2].hist2d(img[:,:,0].flatten(), img[:,:,1].flatten(), bins=256, range=[[0,1],[0,1]])

    plt.savefig(sys.argv[1].replace(".png", "_histogram.png"))


elif sys.argv[2] == "vector3" or sys.argv[2] == "sphere":

    fig, axs = plt.subplots(2,3, figsize=(15, 10))

    axs[0][0].hist(img[:,:,0].flatten(), bins=256, range=(0,1))
    axs[0][1].hist(img[:,:,1].flatten(), bins=256, range=(0,1))
    axs[0][2].hist(img[:,:,2].flatten(), bins=256, range=(0,1))
    axs[1][0].hist2d(img[:,:,0].flatten(), img[:,:,1].flatten(), bins=256, range=[[0,1],[0,1]])
    axs[1][1].hist2d(img[:,:,0].flatten(), img[:,:,2].flatten(), bins=256, range=[[0,1],[0,1]])
    axs[1][2].hist2d(img[:,:,1].flatten(), img[:,:,2].flatten(), bins=256, range=[[0,1],[0,1]])

    plt.savefig(sys.argv[1].replace(".png", "_histogram.png"))


elif sys.argv[2] == "vector4":

    fig, axs = plt.subplots(2,5, figsize=(25, 10))

    axs[0][0].hist(img[:,:,0].flatten(), bins=256, range=(0,1))
    axs[0][1].hist(img[:,:,1].flatten(), bins=256, range=(0,1))
    axs[1][0].hist(img[:,:,2].flatten(), bins=256, range=(0,1))
    axs[1][1].hist(img[:,:,3].flatten(), bins=256, range=(0,1))

    axs[0][2].hist2d(img[:,:,0].flatten(), img[:,:,1].flatten(), bins=256, range=[[0,1],[0,1]])
    axs[1][2].hist2d(img[:,:,0].flatten(), img[:,:,2].flatten(), bins=256, range=[[0,1],[0,1]])
    axs[0][3].hist2d(img[:,:,0].flatten(), img[:,:,3].flatten(), bins=256, range=[[0,1],[0,1]])
    axs[1][3].hist2d(img[:,:,1].flatten(), img[:,:,2].flatten(), bins=256, range=[[0,1],[0,1]])
    axs[0][4].hist2d(img[:,:,1].flatten(), img[:,:,3].flatten(), bins=256, range=[[0,1],[0,1]])
    axs[1][4].hist2d(img[:,:,2].flatten(), img[:,:,3].flatten(), bins=256, range=[[0,1],[0,1]])

    plt.savefig(sys.argv[1].replace(".png", "_histogram.png"))

else:
    print(f"Unimplemented sampleSpace {sys.argv[2]}")
    exit(1)