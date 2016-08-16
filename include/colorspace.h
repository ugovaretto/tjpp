#pragma once
//
// Created by Ugo Varetto on 8/16/16.
//
#include <unordered_map>
#include "Image.h"
#include "JPEGImage.h"

ColorSpace FromTJ(TJPF pf) {
    static std::unordered_map< TJPF, ColorSpace, HashTJPF > tjpfToCS = {
        {TJPF_RGB,  RGB},
        {TJPF_BGR,  BGR},
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

TJPF FromCS(ColorSpace pf) {
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
