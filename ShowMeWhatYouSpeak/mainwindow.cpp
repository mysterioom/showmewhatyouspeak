#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->playstopButton->setIcon(ui->playstopButton->style()->standardIcon(QStyle::SP_MediaPlay));

    audioInput = new AudioInput();
    audioInput->initialize();
    fft = new FFT();
    fft->initialize();
    ui->inputDeviceBox->clear();
    ui->inputDeviceBox->addItems(audioInput->availableAudioDevices());

    connect(ui->playstopButton, &QPushButton::clicked, [&]() -> void {
                emit this->audioDeviceStateChange(isPlayed);
            });

    connect(ui->inputDeviceBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [=](int audioDeviceId){ emit this->audioDeviceChange(audioDeviceId); });

    connect(this, this->audioDeviceChange, audioInput, &AudioInput::setAudioDevice);
    connect(this, this->audioDeviceStateChange, audioInput, &AudioInput::setAudioDeviceState);
    connect(audioInput, &AudioInput::audioDeviceStateChange, this, &MainWindow::audioDeviceStateChanged);
    connect(audioInput, &AudioInput::signalData, fft, &FFT::signalData);   

    image = QImage(imageWidth, imageHeight, QImage::Format_Grayscale8);
    image.fill(Qt::white);
    ui->waterfall->setPixmap(QPixmap::fromImage(image));
}

void MainWindow::audioDeviceStateChanged(bool state) {
    ui->playstopButton->setIcon(style()->standardIcon((state? QStyle::SP_MediaStop : QStyle::SP_MediaPlay)));
    isPlayed = !isPlayed;
}

void MainWindow::drawData(std::shared_ptr<vector<vector<double>>> data) {
    uint size = std::min(static_cast<uint>((*data).size()), imageHeight);
    int offset = std::max(static_cast<int>((*data).size()) - static_cast<int>(imageHeight) - 1, 0);

    for (uint  i = 0; i < imageHeight - size; i++) {
        uchar* pointerFrom = image.scanLine(i + (*data).size());
        uchar* pointerTo = image.scanLine(i);
        for (uint  j = 0; j < imageWidth; j++) {
            *(pointerTo + j) = *(pointerFrom + j);
        }
    }
    for (uint i = 0; i < (*data).size(); i++) {
        uchar* pointer = image.scanLine(imageHeight - size + i);
        uint ratio = ((*data)[offset + i].size() - 1)/(imageWidth - 1);

        for (uint j = 0; j < imageWidth; j++) {
            uint sum = 0;
            for(uint k = 0; k < ratio; k++) {
                int color = (*data)[offset + i][j + k] * 255;
                color = (color > 255 || color < 0 ? 0 : color);
                sum += color;
            }
            *(pointer + j) = QColor(sum/ratio, sum/ratio, sum/ratio).rgb();
        }
    }

    ui->waterfall->setPixmap(QPixmap::fromImage(image));
}

void MainWindow::error(QString error) {
    QMessageBox::critical(this, "Error", error);
}

MainWindow::~MainWindow()
{
    delete ui;
}
