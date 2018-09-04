#include "spectrumanalyser.h"
#include "utils.h"
#include "fftreal_wrapper.h"

#include <qmath.h>
#include <qmetatype.h>
#include <QAudioFormat>
#include <QThread>

SpectrumAnalyserThread::SpectrumAnalyserThread(QObject *parent)
    :   QObject(parent)
    ,   m_fft(new FFTRealWrapper)
    ,   m_numSamples(SpectrumLengthSamples)
    ,   m_window(SpectrumLengthSamples, 0.0)
    ,   m_input(SpectrumLengthSamples, 0.0)
    ,   m_output(SpectrumLengthSamples, 0.0)
    ,   m_spectrum(SpectrumLengthSamples)
{
    calculateWindow();
}

SpectrumAnalyserThread::~SpectrumAnalyserThread()
{
    delete m_fft;
}

void SpectrumAnalyserThread::calculateWindow()
{
    for (int i=0; i<m_numSamples; ++i) {
        DataType x = 0.0;

        //hann window
        x = 0.5 * (1 - qCos((2 * M_PI * i) / (m_numSamples - 1)));

        m_window[i] = x;
    }
}

void SpectrumAnalyserThread::calculateSpectrum(const QByteArray &buffer,
                                                int inputFrequency,
                                                int bytesPerSample)
{
    Q_ASSERT(buffer.size() == m_numSamples * bytesPerSample);

    const char *ptr = buffer.constData();
    for (int i=0; i<m_numSamples; ++i) {
        const qint16 pcmSample = *reinterpret_cast<const qint16*>(ptr);
        // Scale down to range [-1.0, 1.0]
        const DataType realSample = pcmToReal(pcmSample);
        const DataType windowedSample = realSample * m_window[i];
        m_input[i] = windowedSample;
        ptr += bytesPerSample;
    }

    m_fft->calculateFFT(m_output.data(), m_input.data());

    int _baseFrequency = 0;
    int _maxAmplitude = 0;

    for (int i=2; i<=m_numSamples/2; ++i) {
        m_spectrum[i].frequency = qreal(i * inputFrequency) / (m_numSamples);

        const qreal real = m_output[i];
        qreal imag = 0.0;
        if (i>0 && i<m_numSamples/2)
            imag = m_output[m_numSamples/2 + i];

        const qreal magnitude = qSqrt(real*real + imag*imag);
        qreal amplitude = SpectrumAnalyserMultiplier * qLn(magnitude);

        m_spectrum[i].clipped = (amplitude > 1.0);
        amplitude = qMax(qreal(0.0), amplitude);
        amplitude = qMin(qreal(1.0), amplitude);
        m_spectrum[i].amplitude = amplitude;

        if(amplitude > _maxAmplitude){
            _maxAmplitude = amplitude;
            _baseFrequency = m_spectrum[i].frequency;
        }
    }

    emit calculationComplete(m_spectrum, _baseFrequency);
}

SpectrumAnalyser::SpectrumAnalyser(QObject *parent)
    :   QObject(parent)
    ,   m_thread(new SpectrumAnalyserThread(this))
    ,   m_state(Idle)
{
    connect(m_thread, &SpectrumAnalyserThread::calculationComplete,
            this, &SpectrumAnalyser::calculationComplete);
}

SpectrumAnalyser::~SpectrumAnalyser()
{

}


void SpectrumAnalyser::calculate(const QByteArray &buffer,
                         const QAudioFormat &format)
{
    if (isReady()) {
        Q_ASSERT(isPCMS16LE(format));

        const int bytesPerSample = format.sampleSize() * format.channelCount() / 8;

        m_state = Busy;

        const bool b = QMetaObject::invokeMethod(m_thread, "calculateSpectrum",
                                  Qt::AutoConnection,
                                  Q_ARG(QByteArray, buffer),
                                  Q_ARG(int, format.sampleRate()),
                                  Q_ARG(int, bytesPerSample));
        Q_ASSERT(b);
        Q_UNUSED(b);

    }
}

bool SpectrumAnalyser::isReady() const
{
    return (Idle == m_state);
}

void SpectrumAnalyser::cancelCalculation()
{
    if (Busy == m_state)
        m_state = Cancelled;
}

void SpectrumAnalyser::calculationComplete(const FrequencySpectrum &spectrum, int baseFrequency)
{
    Q_ASSERT(Idle != m_state);
    if (Busy == m_state){
        emit spectrumChanged(spectrum);
        emit baseFrequencyChanged(baseFrequency);
    }

    m_state = Idle;
}
