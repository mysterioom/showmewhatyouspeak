#ifndef UTILS_H
#define UTILS_H

#include <QtCore/qglobal.h>
#include <QDebug>

QT_FORWARD_DECLARE_CLASS(QAudioFormat)

const int    AudioSampleRate        = 22050; //Hz
const int    AudioSampleSize         = 16; //bit

qint64 audioDuration(const QAudioFormat &format, qint64 bytes);
qint64 audioLength(const QAudioFormat &format, qint64 microSeconds);

qreal nyquistFrequency(const QAudioFormat &format);

qreal pcmToReal(qint16 pcm);

qint16 realToPcm(qreal real);

bool isPCM(const QAudioFormat &format);

bool isPCMS16LE(const QAudioFormat &format);


template<int N> class PowerOfTwo
{ public: static const int Result = PowerOfTwo<N-1>::Result * 2; };

template<> class PowerOfTwo<0>
{ public: static const int Result = 1; };


#endif // UTILS_H
