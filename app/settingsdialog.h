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


const int MinimumThresholdOfSilence = -60; //dB
const int MaximumThresholdOfSilence = -10; //dB
const int InitThresholdOfSilence = -30; //dB

/**
 * Dialog used to control settings such as the audio input / output device
 * and the windowing function.
 */
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
};

#endif // SETTINGSDIALOG_H
