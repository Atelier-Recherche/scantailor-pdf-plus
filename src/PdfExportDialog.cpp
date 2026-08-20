#include "PdfExportDialog.h"
#include <QFileDialog>
#include <QMessageBox>

PdfExportDialog::PdfExportDialog(QString const& default_path, QWidget* parent)
:   QDialog(parent)
{
    ui.setupUi(this);

    PdfExportOptions const options(PdfExportOptions::fromSettings());
    ui.pathEdit->setText(default_path);
    ui.dpiSpin->setValue(options.dpi);
    ui.qualitySpin->setValue(options.jpegQuality);
    ui.colorCombo->setCurrentIndex(int(options.colorMode));

    connect(ui.browseButton, SIGNAL(clicked()), SLOT(browse()));
    connect(
        ui.colorCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(updateQualityEnabled())
    );
    updateQualityEnabled();
}

QString
PdfExportDialog::outputPath() const
{
    return ui.pathEdit->text().trimmed();
}

PdfExportOptions
PdfExportDialog::options() const
{
    PdfExportOptions options;
    options.dpi = ui.dpiSpin->value();
    options.jpegQuality = ui.qualitySpin->value();
    options.colorMode = static_cast<PdfExportOptions::ColorMode>(
        ui.colorCombo->currentIndex()
    );
    return options;
}

void
PdfExportDialog::browse()
{
    QString path = QFileDialog::getSaveFileName(
        this,
        tr("Export to PDF"),
        outputPath().isEmpty() ? QString() : outputPath(),
        tr("PDF files (*.pdf)")
    );
    if (!path.isEmpty())
    {
        if (!path.endsWith(QString::fromLatin1(".pdf"), Qt::CaseInsensitive))
        {
            path += QString::fromLatin1(".pdf");
        }
        ui.pathEdit->setText(path);
    }
}

void
PdfExportDialog::accept()
{
    if (outputPath().isEmpty())
    {
        QMessageBox::warning(
            this, tr("Export to PDF"),
            tr("Please choose an output file.")
        );
        return;
    }
    options().saveToSettings();
    QDialog::accept();
}

void
PdfExportDialog::updateQualityEnabled()
{
    bool const mono = (ui.colorCombo->currentIndex() == int(PdfExportOptions::ColorMono));
    ui.qualitySpin->setEnabled(!mono);
    ui.qualityLabel->setEnabled(!mono);
}
