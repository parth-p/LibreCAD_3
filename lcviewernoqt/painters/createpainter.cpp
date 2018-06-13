#include "createpainter.h"
#include "openglpainter.h"
#include "lccairopainter.tcc"
LCViewer::LcPainter* createCairoImagePainter(unsigned char* data, const unsigned int width, const unsigned int height) {
    // return new LcCairoPainter<CairoPainter::backend::Image>(data, width, height);
    return new LCViewer::OpenGLPainter(width,height);
}