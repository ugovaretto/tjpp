#pragma once
//
// Created by Ugo Varetto on 8/16/16.
//

//ADD:
// flag support
// pitch: useful to e.g. split and compress in parallel 4k images

#include <turbojpeg.h>

#include "JPEGImage.h"
#include "timing.h"

class TJCompressor {
public:
    TJCompressor() :
        tjCompressor_(tjInitCompress()) {}
    const JPEGImage& Compress(const unsigned char* img,
                              int width,
                              int height,
                              TJPF pf,
                              TJSAMP ss,
                              int quality,
                              int flags = TJFLAG_FASTDCT) {
        if(img_.Empty()
            || UncompressedSize(width, height, pf) > UncompressedSize(img_)) {
            img_.Reset(width, height, pf, ss, quality);
        }
        img_.SetParams(width, height, pf, ss, quality);
        size_t jpegSize = int(UncompressedSize(img_));
        unsigned char* ptr = img_.DataPtr();
#ifdef TIMING__
        Time begin = Tick();
#endif
        if(tjCompress2(tjCompressor_, img, width, 0, height, pf,
                    &ptr, &jpegSize, ss, quality,
                    flags))
            throw std::runtime_error(tjGetErrorStr());
#ifdef TIMING__
        Time end = Tick();
        std::cout << "tjCompress2: "
                  << toms(end - begin).count()
                  << " ms\n";
#endif
        img_.SetJPEGSize(jpegSize);
        return img_;
    }
    //put data back when consumed if possible
    //compressor.Put(std::move(jpegImage));
    void Recycle(JPEGImage&& i) {
        img_ = std::move(i);
    }
    ~TJCompressor() {
        tjDestroy(tjCompressor_);
    }
private:
    JPEGImage img_;
    tjhandle tjCompressor_;
};
