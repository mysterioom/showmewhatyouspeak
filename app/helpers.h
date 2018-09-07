#ifndef HELPERS_H
#define HELPERS_H

#include <QtCore/qglobal.h>
#include <QDebug>

QT_FORWARD_DECLARE_CLASS(QAudioFormat)

const int    AudioSampleRate        = 22050; //Hz
const int    AudioSampleSize         = 16; //bit
const int    AudioChannelsCount      = 1; //recorded in mono

qint64 audioDuration(const QAudioFormat &format, qint64 bytes);
qint64 audioLength(const QAudioFormat &format, qint64 microSeconds);

qreal pcmToReal(qint16 pcm);

qint16 realToPcm(qreal real);

template<int N> class PowOfTwo
{ public: static const int Result = PowOfTwo<N-1>::Result * 2; };

template<> class PowOfTwo<0>
{ public: static const int Result = 1; };


#endif // HELPERS_H
