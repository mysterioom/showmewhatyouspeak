#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QAudio>
#include <QIcon>
#include <QWidget>

class Engine;
class FrequencySpectrum;
class ProgressBar;
class SettingsDialog;
class Spectrograph;
class ToneGeneratorDialog;
class Waveform;

QT_BEGIN_NAMESPACE
class QAction;
class QAudioFormat;
class QLabel;
class QMenu;
class QPushButton;
QT_END_NAMESPACE

/**
 * Main application widget, responsible for connecting the various UI
 * elements to the Engine.
 */
class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = 0);
    ~MainWidget();

    // QObject
    void timerEvent(QTimerEvent *event) override;

public slots:
    void stateChanged(QAudio::Mode mode, QAudio::State state);
    void spectrumChanged(qint64 position, qint64 length,
                         const FrequencySpectrum &spectrum);
    void infoMessage(const QString &message, int timeoutMs);
    void errorMessage(const QString &heading, const QString &detail);
    void displaySilenceLabel(qreal dBLevel);
    void baseFrequencyChanged(int baseFrequency);
    void audioPositionChanged(qint64 position);

private slots:
    void showSettingsDialog();
    void initializeRecord();
    void updateButtonStates();

private:
    void createUi();
    void connectUi();
    void reset();

private:
    Engine*                 m_engine;
    Spectrograph*           m_spectrograph;

    QPushButton*            m_modeButton;
    QPushButton*            m_recordButton;
    QIcon                   m_recordIcon;
    QPushButton*            m_pauseButton;
    QIcon                   m_pauseIcon;
    QPushButton*            m_playButton;
    QIcon                   m_playIcon;
    QPushButton*            m_settingsButton;
    QIcon                   m_settingsIcon;

    QLabel*                 m_infoMessage;
    int                     m_infoMessageTimerId;

    int                     m_thresholdSilence;
    QLabel*                 m_silence;
    QLabel*                 m_basicFrequencyInfoMessage;

    SettingsDialog*         m_settingsDialog;
    ToneGeneratorDialog*    m_toneGeneratorDialog;

    QAction*                m_recordAction;
};

#endif // MAINWIDGET_H
