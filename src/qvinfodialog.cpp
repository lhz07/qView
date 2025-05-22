#include "qvinfodialog.h"
#include "ui_qvinfodialog.h"
#include <QDateTime>
#include <QMimeDatabase>
#include <QTimer>
#include "exif.h"

static int getGcd (int a, int b) {
    return (b == 0) ? a : getGcd(b, a%b);
}

QVInfoDialog::QVInfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QVInfoDialog)
{
    ui->setupUi(this);
    // auto formLayout = static_cast<QFormLayout*>(ui->scrollAreaWidgetContents->layout());
    for (int i = 8; i < ui->formLayout->rowCount(); i++) {
        exif_labels.append(static_cast<QLabel*>(ui->formLayout->itemAt(i, QFormLayout::LabelRole)->widget()));
        exif_fields.append(static_cast<QLabel*>(ui->formLayout->itemAt(i, QFormLayout::FieldRole)->widget()));
    }
    setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint | Qt::CustomizeWindowHint));
    // setFixedSize(0, 0);
    width = 0;
    height = 0;
    frameCount = 0;
}

QVInfoDialog::~QVInfoDialog()
{
    delete ui;
}

void QVInfoDialog::setInfo(const QFileInfo &value, const int &value2, const int &value3, const int &value4)
{
    selectedFileInfo = value;
    width = value2;
    height = value3;
    frameCount = value4;

    // If the dialog is visible, it means we've just navigated to a new image. Instead of running
    // updateInfo immediately, add it to the event queue. This is a workaround for a (Windows-specific?)
    // delay when calling adjustSize on the window if the font contains certain characters (e.g. Chinese)
    // the first time that happens for a given font. At least on Windows, by making the work happen later
    // in the event loop, it allows the main window to repaint first, giving the appearance of better
    // responsiveness. If the dialog is not visible, however, it means we're preparing to display for an
    // image already opened. In this case there is no urgency to repaint the main window, and we need to
    // process the updates here synchronously to avoid the caller showing the dialog before it's ready
    // (i.e. to avoid showing outdated info or placeholder text).
    if (isVisible())
        QTimer::singleShot(0, this, &QVInfoDialog::updateInfo);
    else
        updateInfo();
}

QString format_date(const QString& str)
{
    auto list1 = str.split(" ");
    if (list1.length() == 2){
        return QString("%1 %2").arg(list1[0].replace(":", "/"), list1[1]);
    }else{
        return QString();
    }
}

void QVInfoDialog::updateInfo()
{
    QLocale locale = QLocale::system();
    QMimeDatabase mimedb;
    QMimeType mime = mimedb.mimeTypeForFile(selectedFileInfo.absoluteFilePath(), QMimeDatabase::MatchContent);
    QFile img_file(selectedFileInfo.absoluteFilePath());
    img_file.open(QIODevice::ReadOnly);
    // reset exif labels
    for (auto&& label : exif_fields) {
        label->clear();
    }
    QByteArray q_buf = img_file.readAll();
    auto buf = reinterpret_cast<const unsigned char*>(q_buf.constData());
    easyexif::EXIFInfo exif_info;
    int code = exif_info.parseFrom(buf, q_buf.length());
    if (code == PARSE_EXIF_SUCCESS){
        // show exif labels
        for (int i = 0; i < exif_labels.length(); ++i) {
            exif_labels[i]->show();
            exif_fields[i]->show();
        }
        ui->label_camera_make->setText(exif_info.Make.c_str());
        ui->label_camera_model->setText(exif_info.Model.c_str());
        ui->label_software->setText(exif_info.Software.c_str());
        if (exif_info.BitsPerSample) {
            ui->label_bits_per_sample->setText(QString::number(exif_info.BitsPerSample));
        }
        ui->label_description->setText(exif_info.ImageDescription.c_str());
        ui->label_copyright->setText(exif_info.Copyright.c_str());
        ui->l_image_time->setText(format_date(exif_info.DateTime.c_str()));
        ui->label_original_time->setText(format_date(exif_info.DateTimeOriginal.c_str()));
        ui->label_digitize_time->setText(format_date(exif_info.DateTimeDigitized.c_str()));
        ui->l_subsecond_time->setText(exif_info.SubSecTimeOriginal.c_str());
        ui->label_exposure_time->setText(QString("1/%1 s").arg(QString::number((unsigned)(1.0 / exif_info.ExposureTime))));
        ui->label_F_stop->setText(QString("f/%1").arg(QString::number(exif_info.FNumber)));
        ui->label_exposure_program->setText(exposure_program[exif_info.ExposureProgram]);
        ui->label_ISO->setText(QString::number(exif_info.ISOSpeedRatings));
        if (exif_info.SubjectDistance) {
            ui->label_subject_distance->setText(QString::number(exif_info.SubjectDistance));
        }
        ui->label_exposure_bias->setText(QString("%1 EV").arg(QString::number(exif_info.ExposureBiasValue)));
        ui->label_flash_used->setText(exif_info.Flash ? tr("Yes") : tr("No"));
        ui->label_flash_returned_light->setText(flash_returned_light[exif_info.FlashReturnedLight]);
        ui->label_flash_mode->setText(flash_mode[exif_info.FlashMode]);
        ui->l_metering_mode->setText(metering_mode[exif_info.MeteringMode]);
        ui->l_lens_focal_length->setText(QString("%1 mm").arg(QString::number(exif_info.FocalLength)));
        ui->l_35mm_focal_length->setText(QString("%1 mm").arg(QString::number(exif_info.FocalLengthIn35mm)));
        ui->label_gps_latitude->setText(QString("%1° %2\' %3\" %4").arg(QString::number(exif_info.GeoLocation.LatComponents.degrees),
                                                                        QString::number(exif_info.GeoLocation.LatComponents.minutes),
                                                                        QString::number(exif_info.GeoLocation.LatComponents.seconds),
                                                                        exif_info.GeoLocation.LatComponents.direction));
        ui->label_gps_longtitude->setText(QString("%1° %2\' %3\" %4").arg(QString::number(exif_info.GeoLocation.LonComponents.degrees),
                                                                   QString::number(exif_info.GeoLocation.LonComponents.minutes),
                                                                   QString::number(exif_info.GeoLocation.LonComponents.seconds),
                                                                   exif_info.GeoLocation.LonComponents.direction));
        ui->label_gps_altitude->setText(QString("%1 m").arg(QString::number(exif_info.GeoLocation.Altitude)));
        if (exif_info.GeoLocation.DOP) {
            ui->label_gps_precision->setText(QString::number(exif_info.GeoLocation.DOP));
        }
        ui->l_lens_min_focal_length->setText(QString("%1 mm").arg(QString::number(exif_info.LensInfo.FocalLengthMin)));
        ui->l_lens_max_focal_length->setText(QString("%1 mm").arg(QString::number(exif_info.LensInfo.FocalLengthMax)));
        ui->l_lens_F_min->setText(QString("f/%1").arg(QString::number(exif_info.LensInfo.FStopMin)));
        ui->l_lens_F_max->setText(QString("f/%1").arg(QString::number(exif_info.LensInfo.FStopMax)));
        ui->l_lens_make->setText(exif_info.LensInfo.Make.c_str());
        ui->l_lens_model->setText(exif_info.LensInfo.Model.c_str());
    }else{
        // hide exif labels
        for (int i = 0; i < exif_labels.length(); ++i) {
            exif_labels[i]->hide();
            exif_fields[i]->hide();
        }
    }

    //this is just math to figure the megapixels and then round it to the tenths place
    const double megapixels = static_cast<double>(qRound(((static_cast<double>((width*height))))/1000000 * 10 + 0.5)) / 10 ;

    ui->nameLabel->setText(selectedFileInfo.fileName());
    ui->typeLabel->setText(mime.name());
    ui->locationLabel->setText(selectedFileInfo.path());
    ui->sizeLabel->setText(tr("%1 (%2 bytes)").arg(formatBytes(selectedFileInfo.size()), locale.toString(selectedFileInfo.size())));
    ui->modifiedLabel->setText(selectedFileInfo.lastModified().toString(locale.dateTimeFormat()));
    ui->dimensionsLabel->setText(tr("%1 x %2 (%3 MP)").arg(QString::number(width), QString::number(height), QString::number(megapixels)));
    int gcd = getGcd(width,height);
    if (gcd != 0)
        ui->ratioLabel->setText(QString::number(width/gcd) + ":" + QString::number(height/gcd));
    if (frameCount != 0)
    {
        ui->framesLabel2->show();
        ui->framesLabel->show();
        ui->framesLabel->setText(QString::number(frameCount));
    }
    else
    {
        ui->framesLabel2->hide();
        ui->framesLabel->hide();
    }
    int width_margins = ui->verticalLayout->contentsMargins().left() + ui->verticalLayout->contentsMargins().right() + 20;
    int height_margins = ui->verticalLayout->contentsMargins().top() + ui->verticalLayout->contentsMargins().bottom() + 20;
    // this->resize(ui->formLayout->sizeHint().width() + width_margins, ui->formLayout->sizeHint().height() + height_margins);
    this->setFixedSize(std::min(500, ui->formLayout->sizeHint().width() + width_margins), std::min(600, ui->formLayout->sizeHint().height() + height_margins));
    // window()->adjustSize();
}
