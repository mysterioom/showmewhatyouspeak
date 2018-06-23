#ifndef AUDIOINPUT_H
#define AUDIOINPUT_H

#include <QObject>
#include <QAudioInput>
#include <QAudioDeviceInfo>
#include <vector>
#include <memory>

using std::vector;
using std::shared_ptr;

class AudioInput : public QObject {
    Q_OBJECT
public:
    explicit AudioInput(QObject *parent = 0);
    ~AudioInput();
    QStringList availableAudioDevices();

private:
    QAudioInput* input = nullptr;
    QAudioFormat format;
    QList <QAudioDeviceInfo> devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    uint samplesAtOnce = 1; // samples number change depending on load
    uint fftSize = 2048; // Default FFT size
    int step =  1; // Used for tuning samplesAtOnce value
    int counter = 0; // Checking number of errors in time
    int errors = 0; // Number of errors (samplesAtOnce value too small)

    QIODevice* device = nullptr;
    QByteArray buffer;

public slots:
    void initialize();
    void setAudioDevice(uint audioDeviceId);
    void setAudioDeviceState(bool state);

private slots:
    void readSignal();
    void stateChanged(QAudio::State state);

signals:
    void audioDeviceStateChange(bool state);
    void signalData(shared_ptr<vector<vector<float>>> data);
    void error(QString error);
};

#endif
