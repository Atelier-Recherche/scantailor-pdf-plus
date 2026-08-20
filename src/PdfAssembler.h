#ifndef PDFASSEMBLER_H_
#define PDFASSEMBLER_H_

#include <QString>

class OutputFileNameGenerator;
class PageSequence;

class PdfAssembler
{
public:
    static bool exportMultipage(
        PageSequence const& pages,
        OutputFileNameGenerator const& out_generator,
        QString const& output_pdf_path);
};

#endif
