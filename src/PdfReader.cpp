#include "PdfReader.h"
#include "config.h"
#include <QFile>
#include <QFileInfo>
#include <QtGlobal>

#ifdef ENABLE_PDF_INPUT
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QPdfDocument>
#include <QPdfDocumentRenderOptions>
#endif
#endif

bool
PdfReader::canRead(QString const& file_path)
{
#ifdef ENABLE_PDF_INPUT
    QFileInfo const info(file_path);
    if (info.suffix().compare(QString::fromLatin1("pdf"), Qt::CaseInsensitive) != 0)
    {
        return false;
    }

    QFile file(file_path);
    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    return file.peek(5) == "%PDF-";
#else
    Q_UNUSED(file_path);
    return false;
#endif
}

int
PdfReader::pageCount(QString const& file_path)
{
#if defined(ENABLE_PDF_INPUT) && (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QPdfDocument document;
    if (document.load(file_path) != QPdfDocument::Error::None)
    {
        return 0;
    }
    return document.pageCount();
#else
    Q_UNUSED(file_path);
    return 0;
#endif
}

QImage
PdfReader::readImage(QString const& file_path, int const page_num)
{
#if defined(ENABLE_PDF_INPUT) && (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    if (page_num < 0)
    {
        return QImage();
    }

    QPdfDocument document;
    if (document.load(file_path) != QPdfDocument::Error::None)
    {
        return QImage();
    }
    if (page_num >= document.pageCount())
    {
        return QImage();
    }

    QSizeF const points_size(document.pagePointSize(page_num));
    if (points_size.isEmpty())
    {
        return QImage();
    }

    QSize const render_size(
        qMax(1, qRound(points_size.width() * 2.0)),
        qMax(1, qRound(points_size.height() * 2.0))
    );

    QPdfDocumentRenderOptions options;
    return document.render(page_num, render_size, options);
#else
    Q_UNUSED(file_path);
    Q_UNUSED(page_num);
    return QImage();
#endif
}
