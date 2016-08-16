#pragma once
//
// Created by Ugo Varetto on 8/16/16.
//

//ADD:
// flag support
// pitch: useful to e.g. split and compress in parallel 4k images

#include <stdexcept>
#include <turbojpeg.h>

#include "SyncQueue.h"
#include "JPEGImage.h"
#include "timing.h"

class TJMemPoolCompressor {
public:
    class JPEGImageWrapper {
    public:
        JPEGImageWrapper(JPEGImage img,
                         std::shared_ptr< SyncQueue< JPEGImage > > sq)
        : img_(img), queue_(sq) {}
        const JPEGImage& Image() const { return img_; }
        operator const JPEGImage&() { return Image(); }
        void Flush() {
            queue_->Push(std::move(img_));
        }
        ~JPEGImageWrapper() {
            Flush();
        }
    private:
        JPEGImage img_;
        std::shared_ptr< SyncQueue< JPEGImage > > queue_;
    };
public:
    TJMemPoolCompressor(int numBuffers = 0,
                        size_t w = 0,
                        size_t h = 0,
                        TJPF pf = TJPF_RGB,
                        TJSAMP ss = TJSAMP_420,
                        int q = 75,
                        int flags = TJFLAG_FASTDCT) :
        memoryPool_(new SyncQueue< JPEGImage >()),
        tjCompressor_(tjInitCompress()) {
        for(int i = 0; i != numBuffers; ++i) {
            memoryPool_->Push(JPEGImage(w, h, pf, ss, q));
        }
    }
    JPEGImageWrapper Compress(const unsigned char* img,
                              int width,
                              int height,
                              TJPF pf,
                              TJSAMP ss,
                              int quality,
                              int flags = TJFLAG_FASTDCT) {

        JPEGImage i(memoryPool_->Pop(
            JPEGImage(width, height, pf, ss, quality)));

        if(i.Empty()
            || UncompressedSize(width, height, pf) > UncompressedSize(i)) {
            i.Reset(width, height, pf, ss, quality);
        }
        i.SetParams(width, height, pf, ss, quality);
        size_t jpegSize = int(UncompressedSize(i));
        unsigned char* ptr = i.DataPtr();
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
                  << std::endl;
#endif
        i.SetJPEGSize(jpegSize);
        return JPEGImageWrapper(i, memoryPool_ );
    }
    void PutBack(JPEGImage&& im) {
        memoryPool_->Push(std::move(im));
    }
    void PutBack(JPEGImage& im) {
        memoryPool_->Push(std::move(im));
    }
    ~TJMemPoolCompressor() {
        tjDestroy(tjCompressor_);
    }
private:
    std::shared_ptr< SyncQueue< JPEGImage > > memoryPool_;
    tjhandle tjCompressor_;
};
