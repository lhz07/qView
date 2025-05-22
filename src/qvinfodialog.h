#ifndef QVINFODIALOG_H
#define QVINFODIALOG_H

#include <QDialog>
#include <QFileInfo>
#include <QLocale>
#include <QLabel>

namespace Ui {
class QVInfoDialog;
}

class QVInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QVInfoDialog(QWidget *parent = nullptr);
    ~QVInfoDialog();

    void setInfo(const QFileInfo &value, const int &value2, const int &value3, const int &value4);

    void updateInfo();

private:
    Ui::QVInfoDialog *ui;

    QFileInfo selectedFileInfo;
    int width;
    int height;

    int frameCount;
    QList<QLabel*> exif_labels;
    QList<QLabel*> exif_fields;
    const QMap<char, QString>exposure_program = {{0, tr("Not defined")},
                                                  {1, tr("Manual")},
                                                  {2, tr("Normal program")},
                                                  {3, tr("Aperture priority")},
                                                  {4, tr("Shutter priority")},
                                                  {5, tr("Creative program")},
                                                  {6, tr("Action program")},
                                                  {7, tr("Portrait mode")},
                                                  {8, tr("Landscape mode")}};

    const QMap<char, QString>flash_mode = {{0, tr("Unknown")},
                                           {1, tr("Compulsory flash firing")},
                                           {2, tr("Compulsory flash suppression")},
                                           {3, tr("Automatic mode")}};

    const QMap<char, QString>metering_mode = {{1, tr("Average")},
                                              {2, tr("Center-weighted average")},
                                              {3, tr("Spot")},
                                              {4, tr("Multi-spot")},
                                              {5, tr("Multi-segment")}};

    const QMap<char, QString>flash_returned_light = {{0, tr("No strobe return detection function")},
                                                    {1, tr("Reserved")},
                                                    {2, tr("Strobe return light not detected")},
                                                    {3, tr("Strobe return light detected")}};


public:
    // If Qt 5.10 is available, the built-in function will be used--for Qt 5.9, a custom solution will be used
    static QString formatBytes(qint64 bytes)
    {
        QString sizeString;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
        QLocale locale;
        sizeString = locale.formattedDataSize(bytes);
#else
        double size = bytes;

        int reductionAmount = 0;
        double newSize = size/1024;
        while (newSize > 1024)
        {
            newSize /= 1024;
            reductionAmount++;
            if (reductionAmount > 2)
                break;
        }

        QString unit;
        switch (reductionAmount)
        {
        case 0: {
            unit = " KiB";
            break;
        }
        case 1: {
            unit = " MiB";
            break;
        }
        case 2: {
            unit = " GiB";
            break;
        }
        case 3: {
            unit = " TiB";
            break;
        }
        }

        sizeString = QString::number(newSize, 'f', 2) + unit;
#endif
        return sizeString;
    }
};

#endif // QVINFODIALOG_H
