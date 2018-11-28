from matplotlib import pyplot as plt
import numpy as np
from mpl_toolkits.mplot3d import Axes3D

txt_path_spa = "convolutionSpatialDomain"
txt_path_fre = "convolutionFrequencyDomain"
pixels = [128, 256, 512, 768, 1024, 1536, 2048]
data = []
filter_size = []
number_pixels = []

for i in range(0,20):
    filter_size.append(4 * i + 5)

for i in range(len(pixels)):
    number_pixels.append(pixels[i] * pixels[i])

fig = plt.figure()
ax = Axes3D(fig)
X = filter_size
Y = number_pixels
X, Y = np.meshgrid(X, Y)
Z = np.zeros((7,20))
for i in range(7):
    txt_path = txt_path_fre + " - " + str(pixels[i]) + ".txt"
    data = np.loadtxt(txt_path)
    for j in range(20):
        Z[i,j] = data[j]

ax.plot_surface(X, Y, Z, rstride=1, cstride=1, cmap='rainbow')
ax.set_xlabel("filter size")
ax.set_ylabel("number of pixels")
ax.set_zlabel("time")
ax.ticklabel_format(axis = 'y', style = 'sci', scilimits = (0,0))
ax.yaxis.major.formatter._useMathText = True
plt.show()
