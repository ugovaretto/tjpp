#pragma once
//
// Created by Ugo Varetto on 8/16/16.
//

#include <stdexcept>
#include <turbojpeg.h>

#include "Image.h"
#include "colorspace.h"
#include "timing.h"

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
                     size_t size, int flags = TJFLAG_FASTDCT) {
        int width = -1;
        int height = -1;
        int jpegSubsamp = -1;
        int colorSpace = -1;
        if(tjDecompressHeader3(tjDeCompressor_, jpgImg,
                            size, &width, &height, &jpegSubsamp, &colorSpace))
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
                      width, 0, height, colorSpace, flags))
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
