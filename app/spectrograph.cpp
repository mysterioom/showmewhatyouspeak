#include "spectrograph.h"
#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QTimerEvent>

const int NullTimerId = -1;
const int NullIndex = -1;
const int BarSelectionInterval = 2000;

Spectrograph::Spectrograph(QWidget *parent)
    :   QWidget(parent)
    ,   m_barSelected(NullIndex)
    ,   m_timerId(NullTimerId)
    ,   m_lowFreq(0.0)
    ,   m_highFreq(0.0)
{
    setMinimumHeight(300);
}

Spectrograph::~Spectrograph()
{

}

void Spectrograph::setParams(int numBars, qreal lowFreq, qreal highFreq)
{
    m_bars.resize(numBars);
    m_lowFreq = lowFreq;
    m_highFreq = highFreq;
    updateBars();
}

void Spectrograph::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    killTimer(m_timerId);
    m_timerId = NullTimerId;
    m_barSelected = NullIndex;
    update();
}

void Spectrograph::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);

    const int numBars = m_bars.count();

    if (m_barSelected != NullIndex && numBars) {
        QRect regionRect = rect();
        regionRect.setLeft(m_barSelected * rect().width() / numBars);
        regionRect.setWidth(rect().width() / numBars);
        QColor regionColor(115, 115, 115);
        painter.setBrush(Qt::DiagCrossPattern);
        painter.fillRect(regionRect, regionColor);
        painter.setBrush(Qt::NoBrush);
    }

    QColor barColor(230, 230, 230);

    const QColor gridColor = barColor.darker();
    QPen gridPen(gridColor);
    painter.setPen(gridPen);
    painter.drawLine(rect().topLeft(), rect().topRight());
    painter.drawLine(rect().topRight(), rect().bottomRight());
    painter.drawLine(rect().bottomRight(), rect().bottomLeft());
    painter.drawLine(rect().bottomLeft(), rect().topLeft());

    QVector<qreal> dashes;
    dashes << 2 << 2;
    gridPen.setDashPattern(dashes);
    painter.setPen(gridPen);

    if (numBars) {
        const int numHorizontalSections = numBars;
        QLine line(rect().topLeft(), rect().bottomLeft());
        for (int i=1; i<numHorizontalSections; ++i) {
            line.translate(rect().width()/numHorizontalSections, 0);
            painter.drawLine(line);
        }
    }

    // Draw horizontal lines
    const int numVerticalSections = 10;
    QLine line(rect().topLeft(), rect().topRight());
    for (int i=1; i<numVerticalSections; ++i) {
        line.translate(0, rect().height()/numVerticalSections);
        painter.drawLine(line);
    }

    barColor = barColor.lighter();
    barColor.setAlphaF(0.75);

    if (numBars) {
        const int widgetWidth = rect().width();
        const int barPlusGapWidth = widgetWidth / numBars;
        const int barWidth = 0.8 * barPlusGapWidth;
        const int gapWidth = barPlusGapWidth - barWidth;
        const int paddingWidth = widgetWidth - numBars * (barWidth + gapWidth);
        const int leftPaddingWidth = (paddingWidth + gapWidth) / 2;
        const int barHeight = rect().height() - 2 * gapWidth;

        for (int i=0; i<numBars; ++i) {
            const qreal value = m_bars[i].value;
            QRect bar = rect();
            bar.setLeft(rect().left() + leftPaddingWidth + (i * (gapWidth + barWidth)));
            bar.setWidth(barWidth);
            bar.setTop(rect().top() + gapWidth + (1.0 - value) * barHeight);
            bar.setBottom(rect().bottom() - gapWidth);

            QColor color = barColor;
            painter.fillRect(bar, color);
        }
    }
}

void Spectrograph::mousePressEvent(QMouseEvent *event)
{
    const QPoint pos = event->pos();
    const int index = m_bars.count() * (pos.x() - rect().left()) / rect().width();
    selectBar(index);
}

void Spectrograph::reset()
{
    m_spectrum.reset();
    spectrumChanged(m_spectrum);
}

void Spectrograph::spectrumChanged(const FrequencySpectrum &spectrum)
{
    m_spectrum = spectrum;
    updateBars();
}

int Spectrograph::barIndex(qreal frequency) const
{
    const qreal bandWidth = (m_highFreq - m_lowFreq) / m_bars.count();
    const int index = (frequency - m_lowFreq) / bandWidth;
    return index;
}

QPair<qreal, qreal> Spectrograph::barRange(int index) const
{
    const qreal bandWidth = (m_highFreq - m_lowFreq) / m_bars.count();
    return QPair<qreal, qreal>(index * bandWidth, (index+1) * bandWidth);
}

void Spectrograph::updateBars()
{
    m_bars.fill(Bar());
    FrequencySpectrum::const_iterator i = m_spectrum.begin();
    const FrequencySpectrum::const_iterator end = m_spectrum.end();
    for ( ; i != end; ++i) {
        const FrequencySpectrum::Element e = *i;
        if (e.frequency >= m_lowFreq && e.frequency < m_highFreq) {
            Bar &bar = m_bars[barIndex(e.frequency)];
            bar.value = qMax(bar.value, e.amplitude);
            bar.clipped |= e.clipped;
        }
    }
    update();
}

void Spectrograph::selectBar(int index) {
    const QPair<qreal, qreal> frequencyRange = barRange(index);
    const QString message = QString("Selected frequencies: %1 - %2 Hz")
                                .arg(frequencyRange.first)
                                .arg(frequencyRange.second);
    emit infoMessage(message, BarSelectionInterval);

    if (NullTimerId != m_timerId)
        killTimer(m_timerId);
    m_timerId = startTimer(BarSelectionInterval);

    m_barSelected = index;
    update();
}


