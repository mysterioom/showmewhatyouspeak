#include <QAudioFormat>
#include "helpers.h"

qint64 audioDuration(const QAudioFormat &format, qint64 bytes)
{
    return (bytes * 1000000) /
        (format.sampleRate() * format.channelCount() * (format.sampleSize() / 8));
}

qint64 audioLength(const QAudioFormat &format, qint64 microSeconds)
{
   qint64 result = (format.sampleRate() * format.channelCount() * (format.sampleSize() / 8))
       * microSeconds / 1000000;
   result -= result % (format.channelCount() * format.sampleSize());
   return result;
}

const quint16 PCMS16MaxAmplitude =  32768; // minimum to -32768

qreal pcmToReal(qint16 pcm)
{
    return qreal(pcm) / PCMS16MaxAmplitude;
}
