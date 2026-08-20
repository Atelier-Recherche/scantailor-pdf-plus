#ifndef PDF_EXPORT_DIALOG_H_
#define PDF_EXPORT_DIALOG_H_

#include "ui_PdfExportDialog.h"
#include "PdfAssembler.h"
#include <QDialog>
#include <QString>

class PdfExportDialog : public QDialog
{
    Q_OBJECT
public:
    PdfExportDialog(QString const& default_path, QWidget* parent = 0);

    QString outputPath() const;

    PdfExportOptions options() const;

private slots:
    void browse();

    void accept();

    void updateQualityEnabled();

private:
    Ui::PdfExportDialog ui;
};

#endif
