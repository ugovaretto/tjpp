#pragma once
//Author: Ugo Varetto
//
// This file is part of tjpp.
//tjpp is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
//tjpp is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with tjpp.  If not, see <http://www.gnu.org/licenses/>.

#include <stdexcept>
#include <turbojpeg.h>

#include "Image.h"
#include "colorspace.h"
#include "timing.h"

namespace tjpp {
class TJDeCompressor {
public:
    TJDeCompressor(size_t preAllocatedSize = 0) :
        tjDeCompressor_(tjInitDecompress()) {
        if(preAllocatedSize > 0) {
            img_.Allocate(preAllocatedSize);
        }
    }
    //read data from header case
    Image DeCompress(const unsigned char* jpgImg,
                     size_t size,
                     int flags = TJFLAG_FASTDCT,
                     int pitch = 0) {
        int width = -1;
        int height = -1;
        int jpegSubsamp = -1;
        int colorSpace = -1;
        if(tjDecompressHeader3(tjDeCompressor_,
                               jpgImg,
                               size,
                               &width,
                               &height,
                               &jpegSubsamp,
                               &colorSpace))
            throw std::runtime_error(tjGetErrorStr());
        const size_t uncompressedSize =
            width * height * NumComponents(TJPF(colorSpace));

        if(img_.AllocatedSize() < uncompressedSize) {
            img_.SetParameters(width, height, FromTJ(TJPF(colorSpace)));
            img_.Allocate(uncompressedSize);
        }
        img_.SetParameters(width, height, FromTJ(TJPF(colorSpace)));
#ifdef TIMING__
        Time begin = Tick();
#endif
        if(tjDecompress2(tjDeCompressor_, jpgImg, size, img_.DataPtr(),
                         width, pitch, height, colorSpace, flags))
            throw std::runtime_error(tjGetErrorStr());
#ifdef TIMING__
        Time end = Tick();
        std::cout << "tjDecompress2: "
                  << toms(end - begin).count()
                  << " ms\n";
#endif
        return std::move(img_);
    }
    //if possible put used data back by calling
    //decompressor.Recycle(std::move(usedImage));
    void Recycle(Image&& img) {
        img_ = std::move(img);
    }
    ~TJDeCompressor() {
        tjDestroy(tjDeCompressor_);
    }
private:
    Image img_;
    tjhandle tjDeCompressor_;
};
}