#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "audioinput.h"
#include "fft.h"
#include <QAudioInput>
#include <QAudioDeviceInfo>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private:
    Ui::MainWindow *ui;
    AudioInput* audioInput = nullptr;
    FFT* fft = nullptr;
    bool isPlayed = true;
    // Data
    QImage image;
    uint imageHeight = 300;
    uint imageWidth = 2048 / 2 + 1;
public slots:
    void audioDeviceStateChanged(bool state);
    void drawData(std::shared_ptr<vector<vector<double>>> data);
    void error(QString error);
signals:
    void audioDeviceChange(uint audioDeviceId);
    void audioDeviceStateChange(bool state);
    void fftSizeChange(uint fftSize);

};

#endif // MAINWINDOW_H
