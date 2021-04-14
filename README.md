### Unbiased Weighted Mean Filter
This project implements the [impulse noise](https://en.wikipedia.org/wiki/Salt-and-pepper_noise) (also known as salt-and-pepper noise) removal algorithm published in paper entitled *A weighted mean filter with spatial-bias elimination for impulse noise removal* (UWMF, [see here](https://doi.org/10.1016/j.dsp.2015.08.012)). UWMF is based on another paper entitled *Interpolation-based impulse noise removal* (IBINR, [see here](https://doi.org/10.1049/iet-ipr.2013.0146)). IBINR basically interpolates the corrupted pixel using inverse Euclidean Distance. UWMF recalculates the weights (based on inverse Manhattan Distance) according to the spatial distribution of corrupted pixels, effectively yielding a more spatially-balanced weight contribution.

### How to Compile
```
cd uwmf
mkdir build && cd build
cmake ..
make -j8
```

### How to Use
#### Restore a Corrupted Image
`./uwmf -i <corrupted image> -w <filtering window size>`

Suggested Filtering Window Sizes:

| Corruption Density | Filtering Window Size |
| ------------------ | --------------------- |
| p < 20             | 1                     |
| 20 <= p < 50       | 2                     |
| 50 <= p < 70       | 3                     |
| 70 <= p < 85       | 4                     |
| 85 <= p < 90       | 5                     |
| 90 >= p            | 6                     |


where _p_ is the corruption density. Also note that edge length of a filtering window with size 1 is 3 (2 * _wsize_ + 1). This ensures an odd edge length.

#### Corrupt an Image with Fixed-Valued Impulse Noise (Salt-and-Pepper Noise)
`./uwmf -m c -i <input image> -d <corruption density>`

#### Simulation
Application of UWMF to well-known benchmark images. Images are first corrupted with various corruption densities, ranging from 0.1 to 0.9, and then restored. The restoration capability of UWMF is measured with SSIM, PSNR and IEF.

`./sim.sh <path to images (def: ./images)> <repeat counter (def: 10)> <output file (def: ./results.txt)>`

### TODO
* Offload simulation to a number of threads
* make sure to use release builds of zlib and libpng

### Lincense
GPLv3. See [LICENSE](https://github.com/cengizkandemir/uwmf/blob/master/LICENSE) for full description.
