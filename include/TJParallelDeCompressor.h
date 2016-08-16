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
//along with tjpp. If not, see <http://www.gnu.org/licenses/>.

#include <stdexcept>
#include <turbojpeg.h>

#include "Image.h"
#include "JPEGImage.h"
#include "colorspace.h"
#include "timing.h"
#include <numeric>

namespace tjpp {

class TJParallelDeCompressor {
public:
    TJParallelDeCompressor(int numStacks, size_t preAllocatedSize = 0) :
        handles_(numStacks), tasks_(numStacks) {
        if(preAllocatedSize > 0) {
            img_.Allocate(preAllocatedSize);
        }
        for(int i = 0; i != numStacks; ++i) {
            handles_[i] = tjInitDecompress();
        }
    }
    //read data from header case
    Image DeCompress(const std::vector< JPEGImage >& jpgImgs,
                     int flags = TJFLAG_FASTDCT) {
        const size_t globalWidth = jpgImgs.front().Width();
        const size_t globalHeight
            = std::accumulate(begin(jpgImgs),
                              end(jpgImgs),
                              size_t(0),
                              [](size_t prev, const JPEGImage& i2) {
                                 return prev + i2.Height();
                              });
        int h = 0;
        int colorSpace = -1;
        for(int i = 0; i != jpgImgs.size(); ++i) {
            int width = -1;
            int height = -1;
            int jpegSubsamp = -1;
            if(tjDecompressHeader3(handles_[i],
                                   jpgImgs[i].DataPtr(),
                                   jpgImgs[i].JPEGSize(),
                                   &width,
                                   &height,
                                   &jpegSubsamp,
                                   &colorSpace))
                throw std::runtime_error(tjGetErrorStr());
            assert(width == globalWidth);
            assert(height == jpgImgs[i].Height());
        }


        const size_t uncompressedSize =
            globalWidth * globalHeight * NumComponents(TJPF(colorSpace));

        if(img_.AllocatedSize() < uncompressedSize) {
            img_.SetParameters(globalWidth, globalHeight, FromTJ(TJPF(colorSpace)));
            img_.Allocate(uncompressedSize);
        }
        img_.SetParameters(globalWidth, globalHeight, FromTJ(TJPF(colorSpace)));

        auto decompress = [](tjhandle handle,
                             const unsigned char* jpgImg,
                             size_t size,
                             unsigned char* out,
                             int w, int h, TJPF cs, int flags) {
            if(tjDecompress2(handle, jpgImg, size, out,
                             w, 0, h, cs, flags))
                throw std::runtime_error(tjGetErrorStr());
        };

        for(int i = 0; i != jpgImgs.size(); ++i) {
            const int offset =
                NumComponents(TJPF(colorSpace)) * i
                    * globalWidth * jpgImgs[i].Height();
            tasks_[i] = std::move(std::async(std::launch::async,
                                        decompress,
                                        handles_[i],
                                        jpgImgs[i].DataPtr(),
                                        jpgImgs[i].JPEGSize(),
                                        img_.DataPtr() + offset,
                                        int(globalWidth),
                                        int(jpgImgs[i].Height()),
                                        TJPF(colorSpace),
                                        flags));

        }
        for(auto& f: tasks_) f.get();
        return std::move(img_);
    }
    //if possible put used data back by calling
    //decompressor.Recycle(std::move(usedImage));
    void Recycle(Image&& img) {
        img_ = std::move(img);
    }
    ~TJParallelDeCompressor() {
        for(auto& h: handles_) tjDestroy(h);
    }
private:
    Image img_;
    std::vector< tjhandle > handles_;
    std::vector< std::future< void > > tasks_;
};
}