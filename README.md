### Unbiased Weighted Mean Filter
This project implements (currently not, hopefully soon) the [impulse noise](https://en.wikipedia.org/wiki/Salt-and-pepper_noise) (also known as salt-and-papper noise) removal algorithm published in paper entitled *A weighted mean filter with spatial-bias elimination for impulse noise removal* (UWMF, [see here](https://doi.org/10.1016/j.dsp.2015.08.012)). UWMF is based on another paper entitled *Interpolation-based impulse noise removal* (IBINR, [see here](https://doi.org/10.1049/iet-ipr.2013.0146)). IBINR basically interpolates the corrupted pixel using inverse Euclidean Distance. UWMF recalculates the weights (based on inverse Manhattan Distance) according to the spatial distribution of corrupted pixels, effectively yielding a more spatially-balanced weight contribution.

This will most likely be a command line tool. I am planning to make a CUDA implementation once this one is finished.

### TODO
* test new option parser
* make sure image.h compiles with clang-8
* make sure to use release builds of zlib and libpng

### Lincense
GPLv3. See [LICENSE](https://github.com/cengizkandemir/uwmf/blob/master/LICENSE) for full description.
