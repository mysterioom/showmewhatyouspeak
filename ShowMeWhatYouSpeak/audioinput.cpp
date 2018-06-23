#include "audioinput.h"
#include <QtEndian>

AudioInput::AudioInput(QObject *parent) : QObject(parent) {
}
AudioInput::~AudioInput() {
    delete input; input = nullptr;
    buffer.clear();
}

QStringList AudioInput::availableAudioDevices() {
    QStringList output;
    for (QAudioDeviceInfo info : devices) {
        output.append(info.deviceName());
    }
    return output;
}

// Public slots
void AudioInput::initialize() {
    format = QAudioDeviceInfo::defaultInputDevice().preferredFormat();
    format.setChannelCount(1);
    format.setSampleSize(32);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::Float);

    setAudioDevice(QAudioDeviceInfo::availableDevices(QAudio::AudioInput).indexOf(QAudioDeviceInfo::defaultInputDevice()));

}

void AudioInput::setAudioDevice(uint audioDeviceId) {
    if (!devices.isEmpty() && audioDeviceId < static_cast<uint>(devices.size())) {
        if (devices[audioDeviceId].isFormatSupported(format)) {
            delete input; input = nullptr;
            device = nullptr;

            input = new QAudioInput(devices[audioDeviceId], format, this);
            connect(input, &QAudioInput::stateChanged, this, &AudioInput::stateChanged);
            emit audioDeviceStateChange(false);
        } else {
           emit error("Format is not supported. Cannot record using that device.");
        }
    }
}
void AudioInput::setAudioDeviceState(bool state) {
    if (input != nullptr) {
        if (state) {
            device = input->start();

            if (device != nullptr) {
                connect(device, &QIODevice::readyRead, this, &AudioInput::readSignal);
                emit audioDeviceStateChange(true);
            } else {
                emit error("Cannot read from device.");
            }
        } else {
            input->stop();
            emit audioDeviceStateChange(false);
        }
    } else {
        emit error("Input device is not properly set");
    }
}
//priv
void AudioInput::readSignal() {
    buffer.append(device->readAll());

    if (static_cast<uint>(buffer.size()) > 3 * 2048 * sizeof(float)) {
        uchar* bufferPointer = reinterpret_cast<uchar*>(buffer.data());
        shared_ptr<vector<vector<float>>> data = std::make_shared<vector<vector<float>>>(samplesAtOnce, vector<float>(fftSize));

        for (uint i = 0; i < samplesAtOnce; i++) {
            for (uint j = 0; j < fftSize; j++) {
                (*data)[i][j] = qFromLittleEndian<float>(bufferPointer + (((i * fftSize) + j) * sizeof(float)));
            }
        }

        buffer.remove(0, 3 * 2048 * sizeof(float));
        emit signalData(data);

        // Tuning samples number
        counter++;
        errors += (buffer.size() > static_cast<int>(10 * fftSize * sizeof(float)) ? +1 : -1);

        if (counter == step) {
            if (abs((errors * 10) / step) > 2) {
                samplesAtOnce = std::max(1U, samplesAtOnce + (errors > 0 ? +1 : -1));
            }
            errors = 0;
            counter = 0;
        }
    }
}
void AudioInput::stateChanged(QAudio::State state) {
    if (state == QAudio::StoppedState && input->error() != QAudio::NoError) {
        emit error("Input device sttoped due to the error nr " + QString::number(static_cast<uint>(input->error())));
        emit audioDeviceStateChange(false);
    }
}
