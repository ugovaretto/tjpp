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
#include <unordered_map>

#include "JPEGImage.h"
#include "Image.h"

namespace tjpp {

//
// Note: DO NOT USE THIS STUFF: turbo jpeg colorspace does not look like this
// To use colorspaces this should be modified as per API docs for TJ colorspaces
//

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

inline int NComp(Colorspace pixelFormat) {
    static std::unordered_map< Colorspace, int, HashCS > pfToInt = {
        {RGB, 3},
        {BGR, 3},
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


inline ColorSpace FromTJ(TJPF pf) {
    static std::unordered_map< TJPF, ColorSpace, HashTJPF > tjpfToCS = {
        {TJPF_RGB, RGB},
        {TJPF_BGR, BGR},
        {TJPF_RGBX, RGBX},
        {TJPF_BGRX, BGRX},
        {TJPF_XRGB, XRGB},
        {TJPF_GRAY, GRAY},
        {TJPF_RGBA, RGBA},
        {TJPF_BGRA, BGRA},
        {TJPF_ABGR, ABGR},
        {TJPF_ARGB, ARGB},
        {TJPF_CMYK, CMYK}
    };
    if(tjpfToCS.find(pf) == tjpfToCS.end()) {
        throw std::domain_error("Invalid pixel format "
                                    + std::to_string(pf));
    }
    return tjpfToCS[pf];
}

inline TJPF FromCS(ColorSpace pf) {
    static std::unordered_map< ColorSpace, TJPF, HashCS > tjpfToCS = {
        {RGB, TJPF_RGB},
        {BGR, TJPF_BGR},
        {RGBX, TJPF_RGBX},
        {BGRX, TJPF_BGRX},
        {XRGB, TJPF_XRGB},
        {GRAY, TJPF_GRAY},
        {RGBA, TJPF_RGBA},
        {BGRA, TJPF_BGRA},
        {ABGR, TJPF_ABGR},
        {ARGB, TJPF_ARGB},
        {CMYK, TJPF_CMYK}
    };
    if(tjpfToCS.find(pf) == tjpfToCS.end()) {
        throw std::domain_error("Invalid pixel format "
                                    + std::to_string(pf));
    }
    return tjpfToCS[pf];
}
}