#include "PdfAssembler.h"
#include "OutputFileNameGenerator.h"
#include "PageSequence.h"
#include "TiffReader.h"
#include <QImage>
#include <QPainter>
#include <QPdfWriter>
#include <QFile>

bool
PdfAssembler::exportMultipage(
    PageSequence const& pages,
    OutputFileNameGenerator const& out_generator,
    QString const& output_pdf_path)
{
    if (pages.numPages() == 0 || output_pdf_path.isEmpty())
    {
        return false;
    }

    QPdfWriter writer(output_pdf_path);
    writer.setResolution(300);
    writer.setCreator(QString::fromLatin1("Scan Tailor Experimental"));

    bool wrote_one_page = false;
    bool first_page = true;
    QPainter painter(&writer);
    if (!painter.isActive())
    {
        return false;
    }

    for (size_t i = 0; i < pages.numPages(); ++i)
    {
        QString const tif_path(out_generator.filePathFor(pages.pageAt(i).id()));
        QFile tif_file(tif_path);
        if (!tif_file.open(QIODevice::ReadOnly))
        {
            continue;
        }
        QImage const tif_image(TiffReader::readImage(tif_file));
        if (tif_image.isNull())
        {
            continue;
        }

        if (!first_page)
        {
            writer.newPage();
        }

        QRect const page_rect(0, 0, writer.width(), writer.height());
        QImage scaled = tif_image.scaled(
            page_rect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation
        );
        QPoint const top_left(
            page_rect.x() + (page_rect.width() - scaled.width()) / 2,
            page_rect.y() + (page_rect.height() - scaled.height()) / 2
        );
        painter.drawImage(top_left, scaled);
        wrote_one_page = true;
        first_page = false;
    }

    painter.end();
    return wrote_one_page;
}
