/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

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

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

// Number of audio samples used to calculate the frequency spectrum
const int    SpectrumLengthSamples  = PowerOfTwo<FFTLengthPowerOfTwo>::Result;

// Number of bands in the frequency spectrum
const int    SpectrumNumBands       = 22;

const qreal  SpectrumLowFreq        = 0.0; // Hz
const qreal  SpectrumHighFreq       = 11000; // Hz

// Waveform window size in microseconds
const qint64 WaveformWindowDuration = 500 * 1000;

// Length of waveform tiles in bytes
// Ideally, these would match the QAudio*::bufferSize(), but that isn't
// available until some time after QAudio*::start() has been called, and we
// need this value in order to initialize the waveform display.
// We therefore just choose a sensible value.
const int   WaveformTileLength      = 4096;

// Fudge factor used to calculate the spectrum bar heights
const qreal SpectrumAnalyserMultiplier = 0.15;

// Disable message timeout
const int   NullMessageTimeout      = -1;



QT_FORWARD_DECLARE_CLASS(QAudioFormat)
QT_FORWARD_DECLARE_CLASS(QThread)

class FFTRealWrapper;

class SpectrumAnalyserThreadPrivate;

/**
 * Implementation of the spectrum analysis which can be run in a
 * separate thread.
 */
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

/**
 * Class which performs frequency spectrum analysis on a window of
 * audio samples, provided to it by the Engine.
 */
class SpectrumAnalyser : public QObject
{
    Q_OBJECT

public:
    SpectrumAnalyser(QObject *parent = 0);
    ~SpectrumAnalyser();

public:

    /*
     * Calculate a frequency spectrum
     *
     * \param buffer       Audio data
     * \param format       Format of audio data
     *
     * Frequency spectrum is calculated asynchronously.  The result is returned
     * via the spectrumChanged signal.
     *
     * An ongoing calculation can be cancelled by calling cancelCalculation().
     *
     */
    void calculate(const QByteArray &buffer, const QAudioFormat &format);

    /*
     * Check whether the object is ready to perform another calculation
     */
    bool isReady() const;

    /*
     * Cancel an ongoing calculation
     *
     * Note that cancelling is asynchronous.
     */
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

