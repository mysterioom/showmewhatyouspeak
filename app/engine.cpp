#include "engine.h"
#include "helpers.h"

#include <math.h>

#include <QAudioInput>
#include <QAudioOutput>
#include <QCoreApplication>
#include <QDebug>
#include <QMetaObject>
#include <QSet>
#include <QThread>

const qint64 BufferDurationUs       = 60 * 1000000; //60 seconds
const int    NotifyIntervalMs       = 100;
const int    LevelWindowUs          = 0.1 * 1000000;

Engine::Engine(QObject *parent)
    :   QObject(parent)
    ,   m_mode(QAudio::AudioInput)
    ,   m_state(QAudio::StoppedState)
    ,   m_availableAudioInputDevices
            (QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
    ,   m_audioInputDevice(QAudioDeviceInfo::defaultInputDevice())
    ,   m_audioInput(0)
    ,   m_audioInputIODevice(0)
    ,   m_recordPosition(0)
    ,   m_availableAudioOutputDevices
            (QAudioDeviceInfo::availableDevices(QAudio::AudioOutput))
    ,   m_audioOutputDevice(QAudioDeviceInfo::defaultOutputDevice())
    ,   m_audioOutput(0)
    ,   m_playPosition(0)
    ,   m_bufferPosition(0)
    ,   m_bufferLength(0)
    ,   m_dataLength(0)
    ,   m_levelBufferLength(0)
    ,   m_rmsLevel(0.0)
    ,   m_peakLevel(0.0)
    ,   m_spectrumBufferLength(0)
    ,   m_spectrumAnalyser()
    ,   m_spectrumPosition(0)
    ,   m_count(0)
    ,   m_thresholdSilence(-30)
{
    qRegisterMetaType<FrequencySpectrum>("FrequencySpectrum");
    connect(&m_spectrumAnalyser, QOverload<const FrequencySpectrum&>::of(&SpectrumAnalyser::spectrumChanged),
            this, QOverload<const FrequencySpectrum&>::of(&Engine::spectrumChanged));
    connect(&m_spectrumAnalyser, &SpectrumAnalyser::baseFrequencyChanged,
            this, &Engine::baseFrequencyChanged);

    QStringList arguments = QCoreApplication::instance()->arguments();
    for (int i = 0; i < arguments.count(); ++i) {
        if (arguments.at(i) == QStringLiteral("--"))
            break;

        if (arguments.at(i) == QStringLiteral("-category")
                || arguments.at(i) == QStringLiteral("--category")) {
            ++i;
            if (i < arguments.count())
                m_audioOutputCategory = arguments.at(i);
            else
                --i;
        }
    }

    initialize();
}

Engine::~Engine()
{

}

bool Engine::initializeRecord()
{
    reset();
    return initialize();
}

qint64 Engine::bufferLength() const
{
    return m_bufferLength;
}

void Engine::startRecording()
{
    if (m_audioInput) {
        if (QAudio::AudioInput == m_mode &&
            QAudio::SuspendedState == m_state) {
            m_audioInput->resume();
        } else {
            m_spectrumAnalyser.cancelCalculation();
            spectrumChanged(0, 0, FrequencySpectrum());

            m_buffer.fill(0);
            setRecordPosition(0, true);
            stopPlayback();
            m_mode = QAudio::AudioInput;
            connect(m_audioInput, &QAudioInput::stateChanged,
                    this, &Engine::audioStateChanged);
            connect(m_audioInput, &QAudioInput::notify,
                    this, &Engine::audioNotify);

            m_count = 0;
            m_dataLength = 0;
            emit dataLengthChanged(0);
            m_audioInputIODevice = m_audioInput->start();
            connect(m_audioInputIODevice, &QIODevice::readyRead,
                    this, &Engine::audioDataReady);
        }
    }
}

void Engine::startPlayback()
{
    if (m_audioOutput) {
        if (QAudio::AudioOutput == m_mode &&
            QAudio::SuspendedState == m_state) {
            m_audioOutput->suspend();
            m_audioOutput->resume();
        } else {
            m_spectrumAnalyser.cancelCalculation();
            spectrumChanged(0, 0, FrequencySpectrum());
            setPlayPosition(0, true);
            stopRecording();
            m_mode = QAudio::AudioOutput;
            connect(m_audioOutput, &QAudioOutput::stateChanged,
                    this, &Engine::audioStateChanged);
            connect(m_audioOutput, &QAudioOutput::notify,
                    this, &Engine::audioNotify);

            m_count = 0;
            m_audioOutputIODevice.close();
            m_audioOutputIODevice.setBuffer(&m_buffer);
            m_audioOutputIODevice.open(QIODevice::ReadOnly);
            m_audioOutput->start(&m_audioOutputIODevice);
        }
    }
}

void Engine::suspend()
{
    if (QAudio::ActiveState == m_state ||
        QAudio::IdleState == m_state) {
        switch (m_mode) {
        case QAudio::AudioInput:
            m_audioInput->suspend();
            break;
        case QAudio::AudioOutput:
            m_audioOutput->suspend();
            break;
        }
    }
}

void Engine::setAudioInputDevice(const QAudioDeviceInfo &device)
{
    if (device.deviceName() != m_audioInputDevice.deviceName()) {
        m_audioInputDevice = device;
        initialize();
    }
}

void Engine::setAudioOutputDevice(const QAudioDeviceInfo &device)
{
    if (device.deviceName() != m_audioOutputDevice.deviceName()) {
        m_audioOutputDevice = device;
        initialize();
    }
}

void Engine::setThresholdOfSilence(const int &value)
{
    m_thresholdSilence = value;
}

void Engine::audioNotify()
{
    switch (m_mode)
    {
        case QAudio::AudioInput: {
                const qint64 recordPosition = qMin(m_bufferLength, audioLength(m_format, m_audioInput->processedUSecs()));
                setRecordPosition(recordPosition);
                const qint64 levelPosition = m_dataLength - m_levelBufferLength;
                if (levelPosition >= 0)
                    calculateLevel(levelPosition, m_levelBufferLength);
                if (m_dataLength >= m_spectrumBufferLength) {
                    const qint64 spectrumPosition = m_dataLength - m_spectrumBufferLength;
                    calculateSpectrum(spectrumPosition);
                }
                emit bufferChanged(0, m_dataLength, m_buffer);
            }
            break;
        case QAudio::AudioOutput: {
                const qint64 playPosition = audioLength(m_format, m_audioOutput->processedUSecs());
                setPlayPosition(qMin(bufferLength(), playPosition));
                const qint64 levelPosition = playPosition - m_levelBufferLength;
                const qint64 spectrumPosition = playPosition - m_spectrumBufferLength;
                if (playPosition >= m_dataLength)
                    stopPlayback();
                if (levelPosition >= 0 && levelPosition + m_levelBufferLength < m_bufferPosition + m_dataLength)
                    calculateLevel(levelPosition, m_levelBufferLength);
                if (spectrumPosition >= 0 && spectrumPosition + m_spectrumBufferLength < m_bufferPosition + m_dataLength)
                    calculateSpectrum(spectrumPosition);
            }
            break;
        }
}

void Engine::audioStateChanged(QAudio::State state)
{
    if (QAudio::IdleState == state) {
        stopPlayback();
    } else {
        if (QAudio::StoppedState == state) {
            QAudio::Error error = QAudio::NoError;
            switch (m_mode) {
            case QAudio::AudioInput:
                error = m_audioInput->error();
                break;
            case QAudio::AudioOutput:
                error = m_audioOutput->error();
                break;
            }
            if (QAudio::NoError != error) {
                reset();
                return;
            }
        }
        setState(state);
    }
}

void Engine::audioDataReady()
{
    const qint64 bytesReady = m_audioInput->bytesReady();
    const qint64 bytesSpace = m_buffer.size() - m_dataLength;
    const qint64 bytesToRead = qMin(bytesReady, bytesSpace);

    const qint64 bytesRead = m_audioInputIODevice->read(
                                       m_buffer.data() + m_dataLength,
                                       bytesToRead);

    if (bytesRead) {
        m_dataLength += bytesRead;
        emit dataLengthChanged(dataLength());
    }

    if (m_buffer.size() == m_dataLength)
        stopRecording();
}

void Engine::spectrumChanged(const FrequencySpectrum &spectrum)
{
    emit spectrumChanged(m_spectrumPosition, m_spectrumBufferLength, spectrum);
}

void Engine::resetAudioDevices()
{
    delete m_audioInput;
    m_audioInput = 0;
    m_audioInputIODevice = 0;
    setRecordPosition(0);
    delete m_audioOutput;
    m_audioOutput = 0;
    setPlayPosition(0);
    m_spectrumPosition = 0;
}

void Engine::reset()
{
    stopRecording();
    stopPlayback();
    setState(QAudio::AudioInput, QAudio::StoppedState);
    setFormat(QAudioFormat());
    m_buffer.clear();
    m_bufferPosition = 0;
    m_bufferLength = 0;
    m_dataLength = 0;
    emit dataLengthChanged(0);
    resetAudioDevices();
}

bool Engine::initialize()
{
    bool result = false;

    QAudioFormat format = m_format;

    if (selectFormat()) {
        if (m_format != format) {
            resetAudioDevices();

            m_bufferLength = audioLength(m_format, BufferDurationUs);
            m_buffer.resize(m_bufferLength);
            m_buffer.fill(0);
            emit bufferLengthChanged(bufferLength());
            emit bufferChanged(0, 0, m_buffer);
            m_audioInput = new QAudioInput(m_audioInputDevice, m_format, this);
            m_audioInput->setNotifyInterval(NotifyIntervalMs);
            result = true;

            m_audioOutput = new QAudioOutput(m_audioOutputDevice, m_format, this);
            m_audioOutput->setNotifyInterval(NotifyIntervalMs);
            m_audioOutput->setCategory(m_audioOutputCategory);
        }
    } else {
      emit errorMessage(tr("No i/o format found"), "");
    }

    return result;
}

bool Engine::selectFormat()
{
    if (QAudioFormat() != m_format) {
        QAudioFormat format = m_format;
        if (m_audioOutputDevice.isFormatSupported(format)) {
            setFormat(format);
        }
    } else {

        QList<int> channelsList;
        channelsList += m_audioInputDevice.supportedChannelCounts();
        channelsList += m_audioOutputDevice.supportedChannelCounts();
        channelsList = channelsList.toSet().toList();
        qSort(channelsList);

        QAudioFormat format;
        format.setByteOrder(QAudioFormat::LittleEndian);
        format.setCodec("audio/pcm");
        format.setSampleSize(AudioSampleSize);
        format.setSampleType(QAudioFormat::SignedInt);
        format.setSampleRate(AudioSampleRate);
        format.setChannelCount(AudioChannelsCount);

        setFormat(format);
    }

    return true; // I assume, that every input/output device available
                // listening/recording in default audio parameters
}

void Engine::stopRecording()
{
    if (m_audioInput) {
        m_audioInput->stop();
        QCoreApplication::instance()->processEvents();
        m_audioInput->disconnect();
    }
    m_audioInputIODevice = 0;
}

void Engine::stopPlayback()
{
    if (m_audioOutput) {
        m_audioOutput->stop();
        QCoreApplication::instance()->processEvents();
        m_audioOutput->disconnect();
        setPlayPosition(0);
    }
}

void Engine::setState(QAudio::State state)
{
    const bool changed = (m_state != state);
    m_state = state;
    if (changed)
        emit stateChanged(m_mode, m_state);
}

void Engine::setState(QAudio::Mode mode, QAudio::State state)
{
    const bool changed = (m_mode != mode || m_state != state);
    m_mode = mode;
    m_state = state;
    if (changed)
        emit stateChanged(m_mode, m_state);
}

void Engine::setRecordPosition(qint64 position, bool forceEmit)
{
    const bool changed = (m_recordPosition != position);
    m_recordPosition = position;
    if (changed || forceEmit)
        emit recordPositionChanged(m_recordPosition);
}

void Engine::setPlayPosition(qint64 position, bool forceEmit)
{
    const bool changed = (m_playPosition != position);
    m_playPosition = position;
    if (changed || forceEmit)
        emit playPositionChanged(m_playPosition);
}

void Engine::calculateLevel(qint64 position, qint64 length)
{
    qreal peakLevel = 0.0;

    const char *ptr = m_buffer.constData() + position - m_bufferPosition;
    const char *const end = ptr + length;
    while (ptr < end) {
        const qint16 value = *reinterpret_cast<const qint16*>(ptr);
        const qreal fracValue = pcmToReal(value);
        peakLevel = qMax(peakLevel, fracValue);
        ptr += 2;
    }

    qreal dBLevel = float(20.0f * log10(peakLevel));
    emit displaySilenceLabel(dBLevel);
}

void Engine::calculateSpectrum(qint64 position)
{
    if (m_spectrumAnalyser.isReady()) {
        m_spectrumBuffer = QByteArray::fromRawData(m_buffer.constData() + position - m_bufferPosition,
                                                   m_spectrumBufferLength);
        m_spectrumPosition = position;
        m_spectrumAnalyser.calculate(m_spectrumBuffer, m_format);
    }
}

void Engine::setFormat(const QAudioFormat &format)
{
    const bool changed = (format != m_format);
    m_format = format;
    m_levelBufferLength = audioLength(m_format, LevelWindowUs);
    m_spectrumBufferLength = SpectrumLengthSamples *
                            (m_format.sampleSize() / 8) * m_format.channelCount();
    if (changed)
        emit formatChanged(m_format);
}
