#ifndef SPECTRUMANALYSER_H
#define SPECTRUMANALYSER_H


#include <QByteArray>
#include <QObject>
#include <QVector>
#include <qglobal.h>
#include "FFTRealFixLenParam.h"
#include "fftreal_wrapper.h" // For FFTLengthPowerOfTwo
#include "frequencyspectrum.h"
#include "spectrumanalyser.h"
#include "helpers.h"

// number of audio samples used to calculate the freq spectrum
const int    SpectrumLengthSamples  = PowOfTwo<FFTLengthPowerOfTwo>::Result;

// number of bands in the freq spectrum
const int    SpectrumNumBands       = 22;

const qreal  SpectrumLowFreq        = 0.0; // Hz
const qreal  SpectrumHighFreq       = 11000; // Hz

// window size of waveform in microseconds
const qint64 WaveformWindowDuration = 500 * 1000;

// length of waveform tiles in bytes
const int   WaveformTileLength      = 4096;

// fudge(??) factor used to calculate the spectrum bar heights
const qreal SpectrumAnalyserMultiplier = 0.15;

// disable message timeout
const int   NullMessageTimeout      = -1;

QT_FORWARD_DECLARE_CLASS(QAudioFormat)
QT_FORWARD_DECLARE_CLASS(QThread)

class FFTRealWrapper;

class SpectrumAnalyserThread : public QObject
{
    Q_OBJECT

public:
    SpectrumAnalyserThread(QObject *parent);
    ~SpectrumAnalyserThread();

public slots:
    void calculateSpectrum(const QByteArray &buffer,
                           int inputFrequency,
                           int bytesPerSample);

signals:
    void calculationComplete(const FrequencySpectrum &spectrum, int baseFrequency);

private:
    void calculateWindow();

private:
    FFTRealWrapper*                             m_fft;

    const int                                   m_numSamples;

    typedef FFTRealFixLenParam::DataType        DataType;
    QVector<DataType>                           m_window;

    QVector<DataType>                           m_input;
    QVector<DataType>                           m_output;

    FrequencySpectrum                           m_spectrum;


};

class SpectrumAnalyser : public QObject
{
    Q_OBJECT

public:
    SpectrumAnalyser(QObject *parent = 0);
    ~SpectrumAnalyser();

public:
    void calculate(const QByteArray &buffer, const QAudioFormat &format);
    bool isReady() const;
    void cancelCalculation();

signals:
    void spectrumChanged(const FrequencySpectrum &spectrum);
    void baseFrequencyChanged(int baseFrequency);

private slots:
    void calculationComplete(const FrequencySpectrum &spectrum, int baseFrequency);

private:
    void calculateWindow();

private:

    SpectrumAnalyserThread*    m_thread;

    enum State {
        Idle,
        Busy,
        Cancelled
    };

    State              m_state;
};

#endif // SPECTRUMANALYSER_H

