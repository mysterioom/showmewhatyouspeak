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
#include "utils.h"

// Number of audio samples used to calculate the frequency spectrum
const int    SpectrumLengthSamples  = PowerOfTwo<FFTLengthPowerOfTwo>::Result;

// Number of bands in the frequency spectrum
const int    SpectrumNumBands       = 22;

const qreal  SpectrumLowFreq        = 0.0; // Hz
const qreal  SpectrumHighFreq       = 11000; // Hz

// Waveform window size in microseconds
const qint64 WaveformWindowDuration = 500 * 1000;

// Length of waveform tiles in bytes
const int   WaveformTileLength      = 4096;

// Fudge factor used to calculate the spectrum bar heights
const qreal SpectrumAnalyserMultiplier = 0.15;

// Disable message timeout
const int   NullMessageTimeout      = -1;

QT_FORWARD_DECLARE_CLASS(QAudioFormat)
QT_FORWARD_DECLARE_CLASS(QThread)

class FFTRealWrapper;

class SpectrumAnalyserThreadPrivate;

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

