#include "PdfAssembler.h"
#include "OutputFileNameGenerator.h"
#include "PageSequence.h"
#include "TiffReader.h"
#include <QImage>
#include <QPainter>
#include <QPdfWriter>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QBuffer>
#include <QPageSize>
#include <QSettings>
#include <QSizeF>

PdfExportOptions
PdfExportOptions::defaults()
{
    return PdfExportOptions();
}

PdfExportOptions
PdfExportOptions::fromSettings()
{
    QSettings settings;
    PdfExportOptions options;
    options.dpi = settings.value(QString::fromLatin1("pdf_export/dpi"), 300).toInt();
    options.jpegQuality = settings.value(QString::fromLatin1("pdf_export/quality"), 85).toInt();
    options.colorMode = static_cast<ColorMode>(
        settings.value(QString::fromLatin1("pdf_export/color_mode"), int(ColorAuto)).toInt()
    );

    if (options.dpi < 72)
    {
        options.dpi = 72;
    }
    if (options.dpi > 1200)
    {
        options.dpi = 1200;
    }
    if (options.jpegQuality < 1)
    {
        options.jpegQuality = 1;
    }
    if (options.jpegQuality > 100)
    {
        options.jpegQuality = 100;
    }
    if (options.colorMode < ColorAuto || options.colorMode > ColorMono)
    {
        options.colorMode = ColorAuto;
    }
    return options;
}

void
PdfExportOptions::saveToSettings() const
{
    QSettings settings;
    settings.setValue(QString::fromLatin1("pdf_export/dpi"), dpi);
    settings.setValue(QString::fromLatin1("pdf_export/quality"), jpegQuality);
    settings.setValue(QString::fromLatin1("pdf_export/color_mode"), int(colorMode));
}

namespace
{

QImage
prepareImage(QImage image, PdfExportOptions const& options)
{
    switch (options.colorMode)
    {
    case PdfExportOptions::ColorGrayscale:
        if (image.format() != QImage::Format_Grayscale8)
        {
            image = image.convertToFormat(QImage::Format_Grayscale8);
        }
        break;
    case PdfExportOptions::ColorMono:
        if (image.format() != QImage::Format_Mono && image.format() != QImage::Format_MonoLSB)
        {
            image = image.convertToFormat(QImage::Format_Mono);
        }
        return image;
    case PdfExportOptions::ColorAuto:
    default:
        break;
    }

    if (image.format() == QImage::Format_Mono || image.format() == QImage::Format_MonoLSB)
    {
        return image;
    }

    // Re-encode as JPEG to control compression in the PDF.
    if (image.format() == QImage::Format_Grayscale8
        || options.colorMode == PdfExportOptions::ColorGrayscale)
    {
        if (image.format() != QImage::Format_Grayscale8)
        {
            image = image.convertToFormat(QImage::Format_Grayscale8);
        }
    }
    else if (image.format() != QImage::Format_RGB32
             && image.format() != QImage::Format_ARGB32
             && image.format() != QImage::Format_RGB888)
    {
        image = image.convertToFormat(QImage::Format_RGB32);
    }

    QByteArray jpeg_bytes;
    QBuffer buffer(&jpeg_bytes);
    buffer.open(QIODevice::WriteOnly);
    if (!image.save(&buffer, "JPEG", options.jpegQuality))
    {
        return image;
    }
    buffer.close();

    QImage jpeg_image;
    if (!jpeg_image.loadFromData(jpeg_bytes, "JPEG"))
    {
        return image;
    }
    return jpeg_image;
}

} // anonymous namespace

QString
PdfAssembler::defaultOutputPath(
    PageSequence const& pages,
    OutputFileNameGenerator const& out_generator)
{
    QString pdf_path(
        QDir(out_generator.outDir()).absoluteFilePath(QString::fromLatin1("output.pdf"))
    );
    if (pages.numPages() > 0)
    {
        QFileInfo const source_file(pages.pageAt(0).imageId().filePath());
        if (source_file.suffix().compare(QString::fromLatin1("pdf"), Qt::CaseInsensitive) == 0)
        {
            pdf_path = QDir(source_file.absolutePath()).absoluteFilePath(
                source_file.completeBaseName() + QString::fromLatin1("-done.pdf")
            );
        }
    }
    return pdf_path;
}

bool
PdfAssembler::exportMultipage(
    PageSequence const& pages,
    OutputFileNameGenerator const& out_generator,
    QString const& output_pdf_path,
    PdfExportOptions const& options,
    QString* error_msg)
{
    if (pages.numPages() == 0)
    {
        if (error_msg)
        {
            *error_msg = QString::fromLatin1("No pages to export.");
        }
        return false;
    }
    if (output_pdf_path.isEmpty())
    {
        if (error_msg)
        {
            *error_msg = QString::fromLatin1("Output path is empty.");
        }
        return false;
    }

    QPdfWriter writer(output_pdf_path);
    writer.setResolution(options.dpi);
    writer.setCreator(QString::fromLatin1("ScanTailor PDF+"));
    writer.setTitle(QFileInfo(output_pdf_path).completeBaseName());

    bool wrote_one_page = false;
    bool first_page = true;
    QPainter painter;
    bool painter_begun = false;

    for (size_t i = 0; i < pages.numPages(); ++i)
    {
        QString const tif_path(out_generator.filePathFor(pages.pageAt(i).id()));
        QFile tif_file(tif_path);
        if (!tif_file.open(QIODevice::ReadOnly))
        {
            continue;
        }
        QImage tif_image(TiffReader::readImage(tif_file));
        if (tif_image.isNull())
        {
            continue;
        }

        tif_image = prepareImage(tif_image, options);

        double const width_mm = tif_image.width() * 25.4 / double(options.dpi);
        double const height_mm = tif_image.height() * 25.4 / double(options.dpi);
        writer.setPageSize(
            QPageSize(QSizeF(width_mm, height_mm), QPageSize::Millimeter)
        );

        if (first_page)
        {
            if (!painter.begin(&writer))
            {
                if (error_msg)
                {
                    *error_msg = QString::fromLatin1("Failed to start PDF writer.");
                }
                return false;
            }
            painter_begun = true;
        }
        else
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

    if (painter_begun)
    {
        painter.end();
    }

    if (!wrote_one_page)
    {
        if (error_msg)
        {
            *error_msg = QString::fromLatin1(
                "No output TIFF pages found. Run the Output stage first."
            );
        }
        return false;
    }
    return true;
}
