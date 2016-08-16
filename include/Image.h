#pragma once
//
// Created by Ugo Varetto on 8/16/16.
//
#include <vector>
#include <unordered_map>

enum ColorSpace {
    RGB = 0,
    RGBA,
    GRAY,
    BGR,
    BGRA,
    ABGR,
    ARGB,
    CMYK,
    RGBX,
    BGRX,
    XRGB,
};

struct HashCS {
    size_t operator()(ColorSpace n) const {
        return std::hash< int >()(int(n));
    }
};

int NComp(ColorSpace pixelFormat) {
    static std::unordered_map< ColorSpace, int, HashCS > pfToInt = {
        {RGB,  3},
        {BGR,  3},
        {GRAY, 1},
        {RGBA, 4},
        {BGRA, 4},
        {ABGR, 4},
        {ARGB, 4},
        {CMYK, 4},
        {RGBX, 4},
        {BGRX, 4},
        {XRGB, 4}
    };
    if(pfToInt.find(pixelFormat) == pfToInt.end()) {
        throw std::domain_error("Invalid pixel format "
                                    + std::to_string(pixelFormat));
    }
    return pfToInt[pixelFormat];
}


class Image {
public:
    Image() : width_(0), height_(0), colorSpace_(PixelFormat()) {}
    Image(const std::vector< unsigned char >& data,
          size_t width, size_t height, ColorSpace cs) :
        data_(data), width_(width), height_(height), colorSpace_(cs) {}
    Image(const Image&) = default;
    Image& operator=(const Image&) = default;
    Image(Image&& i) {
        width_ = i.width_;
        height_ = i.height_;
        colorSpace_ = i.colorSpace_;
        data_ = std::move(i.data_);
        i.width_ = 0;
        i.height_ = 0;
    }
    Image& operator=(Image&& i) {
        width_ = i.width_;
        height_ = i.height_;
        colorSpace_ = i.colorSpace_;
        data_ = std::move(i.data_);
        i.width_ = 0;
        i.height_ = 0;
        return *this;
    }
    size_t Width() const { return width_; }
    size_t Height() const { return height_; }
    ColorSpace PixelFormat() const { return colorSpace_; }
    std::vector< unsigned char > Data() const { return data_; }
    const unsigned char* DataPtr() const { return data_.data(); }
    unsigned char* DataPtr() { return data_.data(); }
    int NumComponents() const {
        return NComp(colorSpace_);
    }
    size_t Size() const { return width_ * height_ * NumComponents(); }
    size_t AllocatedSize() const { return data_.size(); }
    void Allocate(size_t sz) {
        data_.resize(sz);
    }
    void SetParameters(size_t w, size_t h, ColorSpace cs) {
        width_ = w;
        height_ = h;
        colorSpace_ = cs;
    }
private:
    size_t width_;
    size_t height_;
    ColorSpace colorSpace_;
    std::vector< unsigned char > data_;
};

