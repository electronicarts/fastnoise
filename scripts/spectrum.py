#///////////////////////////////////////////////////////////////////////////////
#//               FastNoise - F.A.S.T. Sampling Implementation                //
#//         Copyright (c) 2023 Electronic Arts Inc. All rights reserved.      //
#///////////////////////////////////////////////////////////////////////////////

import sys
import numpy as np
from matplotlib import image
from numpy import pi, sin, cos, modf, sqrt
import imageio

# imageio is for hdr support.
# https://matiascodesal.com/blog/how-read-hdr-image-using-python/

if len(sys.argv) != 3:
    print(f"Usage: {sys.argv[0]} filename sampleSpace")
    print("Where sampleSpace is (real|circle|sphere|vector2|vector3|vector4)")
    exit(1)

# 
filename = sys.argv[1]
sampleSpace = sys.argv[2]
numSamples = 256

img = None

isHDR = filename.endswith(".hdr")

if isHDR:
    imageio.plugins.freeimage.download()
    img = imageio.v2.imread(filename, format='HDR-FI')
else:
    img = image.imread(filename)

# Interpret 2D image as a stack of square images
inputShape = img.shape
fftShape = [img.shape[0]//img.shape[1], img.shape[1], img.shape[1]]
flatShape = [img.shape[0] * img.shape[1], img.shape[2]]
print(f"Dimensions {img.shape}, interpreting as {fftShape}")
img = np.reshape(img, flatShape)

# Tool outputs 4-component images; reduce to just the ones we need
if sampleSpace == "circle" or sampleSpace == "real":
    img = img[:,0]
elif sampleSpace == "vector2":
    img = img[:,0:2]
elif sampleSpace == "sphere" or sampleSpace == "vector3":
    img = img[:,0:3]

# Map from [0,1] into [-1,1]
if not isHDR:
    img = (img - 0.5) * 2.0 * 255.0 / 256.0


# We will use this to accumulate squared spectrum of the noise
meanSquareSpectrum = np.zeros(fftShape)

# 
for s in range(numSamples):

    mask = np.zeros(img.shape[0])

    if sampleSpace == "real":
        
        t = 2*(s + np.random.rand()) / numSamples - 1
        for i in range(img.shape[0]):
            mask[i] = 1.0 if img[i] < t else 0.0

    elif sampleSpace == "circle":
    
        t = 2*(s + np.random.rand()) / numSamples
        for i in range(img.shape[0]):
            mask[i] = 1.0 if (t + img[i]) % 2.0 < 1.0 else 0.0    
            
    elif sampleSpace == "sphere":
    
        # Uniform sampling of sphere
        phi = 2 * pi * np.random.rand()
        u = 2*np.random.rand()-1
        v = np.array([sqrt(1-u**2) * cos(phi), sqrt(1-u**2) * sin(phi), u])
        
        for i in range(img.shape[0]):
            mask[i] = 1.0 if np.dot(v, img[i,:]) < 0.0 else 0.0

    elif sampleSpace == "vector2":

        # Generate random Heaviside function
        phi = 2 * pi * np.random.rand()
        offset = sqrt(2) * (2*np.random.rand()-1)
        v = np.array([cos(phi), sin(phi)])

        for i in range(img.shape[0]):
            mask[i] = 1.0 if np.dot(v, img[i,:]) < offset else 0.0

    elif sampleSpace == "vector3":

        # Generate random Heaviside function
        phi = 2 * pi * np.random.rand()
        u = 2*np.random.rand()-1
        v = np.array([sqrt(1-u**2) * cos(phi), sqrt(1-u**2) * sin(phi), u])
        offset = sqrt(3) * (2*np.random.rand()-1)

        for i in range(img.shape[0]):
            mask[i] = 1.0 if np.dot(v, img[i,:]) < offset else 0.0

    elif sampleSpace == "vector4":

        # Generate random Heaviside function
        u = np.random.normal(0,1,4)  
        v = u / sqrt(np.sum(u**2))
        offset = 2 * (2*np.random.rand()-1)

        for i in range(img.shape[0]):
            mask[i] = 1.0 if np.dot(v, img[i,:]) < offset else 0.0

    else:
        print(f"unknown sample space {samplespace}")
        exit()

    # Average the square of the Fourier transform    
    spectrum = np.fft.fftn(np.reshape(mask, fftShape))
    meanSquareSpectrum += abs(spectrum)**2

    print(f"{s}/{numSamples}", end="\r")


# Output the RMS spectrum, with the zero mode removed
meanSquareSpectrum[0,0,0] = 0
RMSSpectrum = np.sqrt(np.fft.fftshift(meanSquareSpectrum))

# Reshape RMS spectrum to match the input
if isHDR:
    filename = filename.replace(".hdr", "_spectrum.png")
else:
    filename = filename.replace(".png", "_spectrum.png")
image.imsave(filename, np.reshape(RMSSpectrum, [inputShape[0], inputShape[1]]))
