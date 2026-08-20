#ifndef PDFASSEMBLER_H_
#define PDFASSEMBLER_H_

#include <QString>

class OutputFileNameGenerator;
class PageSequence;

struct PdfExportOptions
{
    enum ColorMode
    {
        ColorAuto = 0,
        ColorGrayscale = 1,
        ColorMono = 2
    };

    int dpi = 300;
    int jpegQuality = 85;
    ColorMode colorMode = ColorAuto;

    static PdfExportOptions defaults();
    static PdfExportOptions fromSettings();
    void saveToSettings() const;
};

class PdfAssembler
{
public:
    static QString defaultOutputPath(
        PageSequence const& pages,
        OutputFileNameGenerator const& out_generator);

    static bool exportMultipage(
        PageSequence const& pages,
        OutputFileNameGenerator const& out_generator,
        QString const& output_pdf_path,
        PdfExportOptions const& options = PdfExportOptions::defaults(),
        QString* error_msg = 0);
};

#endif
