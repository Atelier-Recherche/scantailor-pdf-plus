#include "PdfMetadataLoader.h"
#include "PdfReader.h"
#include "ImageMetadata.h"
#include "config.h"
#include <QFile>
#include <QImage>

void
PdfMetadataLoader::registerMyself()
{
    static bool registered = false;
    if (!registered)
    {
        ImageMetadataLoader::registerLoader(
            IntrusivePtr<ImageMetadataLoader>(new PdfMetadataLoader)
        );
        registered = true;
    }
}

ImageMetadataLoader::Status
PdfMetadataLoader::loadMetadata(
    QIODevice& io_device,
    VirtualFunction1<void, ImageMetadata const&>& out)
{
#ifdef ENABLE_PDF_INPUT
    QFile* const file = dynamic_cast<QFile*>(&io_device);
    if (!file)
    {
        return FORMAT_NOT_RECOGNIZED;
    }

    QString const file_path(file->fileName());
    if (!PdfReader::canRead(file_path))
    {
        return FORMAT_NOT_RECOGNIZED;
    }

    int const pages = PdfReader::pageCount(file_path);
    if (pages <= 0)
    {
        return NO_IMAGES;
    }

    for (int page = 0; page < pages; ++page)
    {
        QImage const image(PdfReader::readImage(file_path, page));
        if (image.isNull())
        {
            return GENERIC_ERROR;
        }
        out(ImageMetadata(image.size()));
    }
    return LOADED;
#else
    Q_UNUSED(io_device);
    Q_UNUSED(out);
    return FORMAT_NOT_RECOGNIZED;
#endif
}
