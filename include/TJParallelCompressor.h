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
//along with Foobar.  If not, see <http://www.gnu.org/licenses/>.

//ADD:
// flag support
// pitch: useful to e.g. split and compress in parallel 4k images

#include <thread>
#include <future>
#include <turbojpeg.h>

#include "JPEGImage.h"
#include "timing.h"

namespace tjpp {
template < typename C >
class TJParallelCompressor {
public:
    TJParallelCompressor(int numCompressors) : compressors_(numCompressors) {}
    std::vector< JPEGImage > Compress(const unsigned char* img,
                                      int stacks,
                                      int width,
                                      int height,
                                      TJPF pf,
                                      TJSAMP ss,
                                      int quality,
                                      int offset = 0,
                                      int flags = TJFLAG_FASTDCT,
                                      int pitch = 0) {

        assert(size_t(stacks) <= compressors_.size());
        auto compress = [&](C* compressor, int h, int off) {
            return compressor->Compress(img, width, h, pf, ss,
                                        quality, off, flags, pitch);
        };
        std::vector< std::future< JPEGImage > > tasks_;
        const int h = height / stacks;
        const int nc = NumComponents(pf);
        for(int s = 0; s != stacks - 1; ++s) {
            tasks_.push_back( std::async(std::launch::async, compress,
                           &compressors_[s], h, s * h * width * nc));

        }
        const size_t last = compressors_.size() - 1;
        const int lastOffset = nc * width * (stacks - 1) * h;
        const int lastHeight = height - (stacks - 1) * h;
        tasks_.push_back(
            std::async(std::launch::async, compress,
                       &compressors_[last],
                       lastHeight,
                       lastOffset));
        std::vector< JPEGImage > images;
        for(auto& f: tasks_) {
            images.push_back(f.get());
        }
        return images;

    }
//    //put data back when consumed if possible
//    //compressor.Put(std::move(jpegImage));
//    void Recycle(JPEGImage&& i) {
//        img_ = std::move(i);
//    }
//    ~TJParallelCompressor() {
//        tjDestroy(tjCompressor_);
//    }
private:
    std::vector< C > compressors_;
};
}