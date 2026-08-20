#ifndef PDFREADER_H_
#define PDFREADER_H_

#include <QImage>
#include <QString>

class PdfReader
{
public:
    static bool canRead(QString const& file_path);
    static int pageCount(QString const& file_path);
    static QImage readImage(QString const& file_path, int page_num);
};

#endif
