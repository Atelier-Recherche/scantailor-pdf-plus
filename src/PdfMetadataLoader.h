#ifndef PDFMETADATALOADER_H_
#define PDFMETADATALOADER_H_

#include "ImageMetadataLoader.h"

class PdfMetadataLoader : public ImageMetadataLoader
{
public:
    static void registerMyself();
protected:
    virtual Status loadMetadata(
        QIODevice& io_device,
        VirtualFunction1<void, ImageMetadata const&>& out);
};

#endif
