//
// Created by Ugo Varetto on 8/15/16.
//

#include <vector>
#include <cstring>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "TJCompressor.h"
#include "TJMemPoolCompressor.h"
#include "TJDeCompressor.h"

#ifdef TIMING__
#include "timing.h"
#endif


using namespace std;

size_t FileSize(const string& fname) {
    ifstream file(fname);
    assert(file);
    file.ignore( std::numeric_limits<std::streamsize>::max() );
    std::streamsize length = file.gcount();
    file.clear();   //  Since ignore will have set eof.
    //file.seekg( 0, std::ios_base::beg );
    return length;
}
void TestJPGMemPoolCompressor(const unsigned char* uimg,
                              int width,
                              int height,
                              TJPF pf,
                              TJSAMP ss,
                              int quality,
                              int numImages) {
    TJMemPoolCompressor tjc;
    for(int i = 0; i != numImages; ++i) {
//        TJMemPoolCompressor::JPEGImageWrapper iw = tjc.Compress(uimg,
//                                              width, height, pf, ss, quality);
        //Compress returns a image wrapper which gets automatically converted
        //to an image const ref
        JPEGImage img = tjc.Compress(uimg,
                                     width, height, pf, ss, quality);
        const string fname = "out" + to_string(i) + ".jpg";

        //JPEGImage img = iw.Image();
        ofstream os(fname, ios::binary);
        assert(os);
        assert(img.DataPtr());
        os.write((char*)img.DataPtr(), img.JPEGSize());
        //tjc.PutBack(img);
        assert(img.Empty()); //moved!
    }
}


int TestCompressorAndDecompressor(int argc, char** argv) {
    //read
    if(argc < 3 ){
        cerr << "usage: " << argv[0]
             << " <jpeg file> <quality=[0,100]>" << endl;
    }
    const size_t length = FileSize(argv[1]);
    using Byte = unsigned char;
    vector< Byte > input(length);
    ifstream is(argv[1], ios::binary);
    is.read((char*) input.data(), input.size());
    //Image img;
    //decompress
    TJDeCompressor decomp(1920 * 1080 * 4 * 4); //up to RGBX 4k support
#ifdef TIMING__
    Time begin = Tick();
#endif
    //move semantics for both constructor and assignment operator
    Image img;
    img = (decomp.DeCompress(input.data(), input.size()));
#ifdef TIMING__
    Time end = Tick();
#endif
    //compress
    const int quality = strtol(argv[2], nullptr, 10);
    TJCompressor comp;
#ifdef TIMING__
    Time begin2 = Tick();
#endif
    JPEGImage jpegImage =
        comp.Compress(img.DataPtr(), img.Width(), img.Height(),
                      FromCS(img.PixelFormat()), TJSAMP_420, quality);
#ifdef TIMING__
    Time end2 = Tick();
#endif
    //write
    ofstream os("out.jpg", ios::binary);
    assert(os);
    os.write((const char*) jpegImage.DataPtr(), jpegImage.JPEGSize());
    os.flush();

#ifdef TIMING__
    Duration elapsed = end - begin;
    cout << "Decompression time: "
         << toms(elapsed).count()
         << "ms" << endl;

    Duration elapsed2 = end2 - begin2;
    cout << "Compression time: "
         << toms(elapsed2).count()
         << "ms" << endl;
#endif

    TestJPGMemPoolCompressor(img.DataPtr(), img.Width(), img.Height(),
                             FromCS(img.PixelFormat()), TJSAMP_420, 50, 10);

    return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
   return TestCompressorAndDecompressor(argc, argv);

}