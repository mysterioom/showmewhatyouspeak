#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "spectrumanalyser.h"
#include <QDialog>
#include <QAudioDeviceInfo>

QT_BEGIN_NAMESPACE
class QComboBox;
class QCheckBox;
class QSlider;
class QSpinBox;
class QGridLayout;
class QLabel;
QT_END_NAMESPACE

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    SettingsDialog(const QList<QAudioDeviceInfo> &availableInputDevices,
                   const QList<QAudioDeviceInfo> &availableOutputDevices,
                   const int &thresholdSilence,
                   QWidget *parent = 0);
    ~SettingsDialog();

    const QAudioDeviceInfo &inputDevice() const { return m_inputDevice; }
    const QAudioDeviceInfo &outputDevice() const { return m_outputDevice; }
    const int &thresholdSilence() {return m_thresholdSilence; }
    const int &minimumThresholdOfSilence() {return m_minimumThresholdOfSilence; }
    const int &maximumThresholdOfSilence() {return m_maximumThresholdOfSilence; }
    const int &initThresholdOfSilence() {return m_initThresholdOfSilence; }

private slots:
    void inputDeviceChanged(int index);
    void outputDeviceChanged(int index);
    void thresholdSilenceChanged(int index);

private:
    QAudioDeviceInfo m_inputDevice;
    QAudioDeviceInfo m_outputDevice;

    QComboBox *m_inputDeviceComboBox;
    QComboBox *m_outputDeviceComboBox;
    QSlider *m_thresholdOfSilenceSlider;
    QLabel *m_thresholdOfSilenceValueLabel;
    int m_thresholdSilence;

    const int m_minimumThresholdOfSilence = -60; //dB
    const int m_maximumThresholdOfSilence = -10; //dB
    const int m_initThresholdOfSilence = -30; //dB
};

#endif // SETTINGSDIALOG_H
