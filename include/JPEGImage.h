#pragma once
///Author: Ugo Varetto
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
//along with Foobar.  If not, see <http://www.gnu.org/licenses/>.

//Note: consider using X* or *X pixel format to speed up memory access
#include <memory>
#include <unordered_map>
#include <stdexcept>

#include <turbojpeg.h>

namespace tjpp {
struct HashTJPF {
    size_t operator()(TJPF n) const {
        return std::hash< int >()(int(n));
    }
};
int NumComponents(TJPF tjpgPixelFormat) {
    static std::unordered_map< TJPF, int, HashTJPF > tjpfToInt = {
        {TJPF_RGB, 3},
        {TJPF_BGR, 3},
        {TJPF_RGBX, 4},
        {TJPF_BGRX, 4},
        {TJPF_XRGB, 4},
        {TJPF_GRAY, 1},
        {TJPF_RGBA, 4},
        {TJPF_BGRA, 4},
        {TJPF_ABGR, 4},
        {TJPF_ARGB, 4},
        {TJPF_CMYK, 4}
    };
    if(tjpfToInt.find(tjpgPixelFormat) == tjpfToInt.end()) {
        throw std::domain_error("Invalid pixel format "
                                    + std::to_string(tjpgPixelFormat));
    }
    return tjpfToInt[tjpgPixelFormat];
}

void TJDeleter(unsigned char* ptr) {
    if(!ptr)
        return;
    tjFree(ptr);
}

class JPEGImage {
public:
    JPEGImage() : width_(0), height_(0), pixelFormat_(TJPF()),
                  subSampling_(TJSAMP()), quality_(50) {}
    JPEGImage(const JPEGImage&) = default;
    JPEGImage(JPEGImage&& i) {
        width_ = i.width_;
        height_ = i.height_;
        pixelFormat_ = i.pixelFormat_;
        subSampling_ = i.subSampling_;
        quality_ = i.quality_;
        data_ = std::move(i.data_);
        i.data_.reset();
        i.width_ = 0;
        i.height_ = 0;
    }
    JPEGImage(int w, int h, TJPF pf, TJSAMP s, int q) :
        width_(w), height_(h), pixelFormat_(pf), subSampling_(s), quality_(q),
        data_(tjAlloc(w * h * NumComponents(pf)), TJDeleter) {} //
    JPEGImage& operator=(JPEGImage&& i) {
        width_ = i.width_;
        height_ = i.height_;
        pixelFormat_ = i.pixelFormat_;
        subSampling_ = i.subSampling_;
        quality_ = i.quality_;
        data_ = std::move(i.data_);
        i.width_ = 0;
        i.height_ = 0;
        i.data_.reset();
        return *this;
    }
    int Width() const { return width_; }
    int Height() const { return height_; }
    TJPF PixelFormat() const { return pixelFormat_; }
    TJSAMP ChrominanceSubSampling() const { return subSampling_; }
    std::shared_ptr< unsigned char > Data() const { return data_; }
    unsigned char* DataPtr() {
        return data_.get();
    }
    bool Empty() const {
        return bool(data_);
    }
    void Reset(int w, int h, TJPF pf, TJSAMP s, int quality) {
        SetParams(w, h, pf, s, quality);
        const size_t sz = std::max(tjBufSize(w, h, s),
                                   static_cast< unsigned long >(
                                       w * h * NumComponents(pf)));
        data_.reset(tjAlloc(sz),
                    TJDeleter);
    }
    void SetParams(size_t w, size_t h, TJPF pf, TJSAMP ss, int q) {
        width_ = w;
        height_ = h;
        pixelFormat_ = pf;
        subSampling_ = ss;
        quality_ = q;
    }
    void SetJPEGSize(size_t s) { jpegSize_ = s; }
    size_t JPEGSize() const { return jpegSize_; }
    bool operator!() const { return Empty(); }
private:
    int width_;
    int height_;
    TJPF pixelFormat_;
    TJSAMP subSampling_;
    int quality_;
    size_t jpegSize_;
    std::shared_ptr< unsigned char > data_;
};

size_t UncompressedSize(size_t width, size_t height, TJPF pf) {
    return width * height * NumComponents(pf);
}

size_t UncompressedSize(const JPEGImage& i) {
    return size_t(i.Width() * i.Height() * NumComponents(i.PixelFormat()));
}
}