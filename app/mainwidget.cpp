#include "engine.h"
#include "mainwidget.h"
#include "settingsdialog.h"
#include "spectrograph.h"
#include "utils.h"

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStyle>
#include <QMenu>
#include <QFileDialog>
#include <QTimerEvent>
#include <QMessageBox>

const int NullTimerId = -1;

MainWidget::MainWidget(QWidget *parent)
    :   QWidget(parent)
    ,   m_engine(new Engine(this))
    ,   m_spectrograph(new Spectrograph(this))
    ,   m_recordButton(new QPushButton(this))
    ,   m_pauseButton(new QPushButton(this))
    ,   m_playButton(new QPushButton(this))
    ,   m_settingsButton(new QPushButton(this))
    ,   m_infoMessage(new QLabel(tr(""), this))
    ,   m_infoMessageTimerId(NullTimerId)
    ,   m_silence(new QLabel(tr("SILENCE"), this))
    ,   m_basicFrequencyInfoMessage(new QLabel(tr("Basic voice frequency: 0 Hz"), this))
    ,   m_settingsDialog(new SettingsDialog(
            m_engine->availableAudioInputDevices(),
            m_engine->availableAudioOutputDevices(),
            m_engine->thresholdSilence(),
            this))
    ,   m_recordAction(0)
{
    m_spectrograph->setParams(SpectrumNumBands, SpectrumLowFreq, SpectrumHighFreq);

    createUi();
    connectUi();
}

MainWidget::~MainWidget()
{

}

void MainWidget::stateChanged(QAudio::Mode mode, QAudio::State state)
{
    Q_UNUSED(mode);

    updateButtonStates();

    if (QAudio::ActiveState != state &&
        QAudio::SuspendedState != state &&
        QAudio::InterruptedState != state) {
        m_spectrograph->reset();
    }
}

void MainWidget::spectrumChanged(qint64 position, qint64 length,
                                 const FrequencySpectrum &spectrum)
{
    Q_UNUSED(position);
    Q_UNUSED(length);
    m_spectrograph->spectrumChanged(spectrum);
}

void MainWidget::infoMessage(const QString &message, int timeoutMs)
{
    m_infoMessage->setText(message);

    if (NullTimerId != m_infoMessageTimerId) {
        killTimer(m_infoMessageTimerId);
        m_infoMessageTimerId = NullTimerId;
    }

    if (NullMessageTimeout != timeoutMs)
        m_infoMessageTimerId = startTimer(timeoutMs);
}

void MainWidget::displaySilenceLabel(qreal dBLevel)
{
    int thresholdSilence = m_engine->thresholdSilence();
    if(dBLevel <= thresholdSilence){
        m_silence->setText("SILENCE");
    } else {
        m_silence->setText("");
    }
}

void MainWidget::baseFrequencyChanged(int baseFrequency)
{
    m_basicFrequencyInfoMessage->setText(QString("Basic voice frequency: %1 Hz")
                                         .arg(baseFrequency));
}

void MainWidget::errorMessage(const QString &heading, const QString &detail)
{
    QMessageBox::warning(this, heading, detail, QMessageBox::Close);
}

void MainWidget::timerEvent(QTimerEvent *event)
{
    Q_ASSERT(event->timerId() == m_infoMessageTimerId);
    Q_UNUSED(event)
    killTimer(m_infoMessageTimerId);
    m_infoMessageTimerId = NullTimerId;
    m_infoMessage->setText("");
}

void MainWidget::audioPositionChanged(qint64 position)
{
    Q_UNUSED(position)
}

void MainWidget::showSettingsDialog()
{
    m_settingsDialog->exec();
    if (m_settingsDialog->result() == QDialog::Accepted) {
        m_engine->setAudioInputDevice(m_settingsDialog->inputDevice());
        m_engine->setAudioOutputDevice(m_settingsDialog->outputDevice());
        m_engine->setThresholdOfSilence((m_settingsDialog->thresholdSilence()));
    }
}

void MainWidget::initializeRecord()
{
    reset();
    if (m_engine->initializeRecord())
        updateButtonStates();
}

void MainWidget::createUi()
{
    setWindowTitle(tr("Show me, what you speak!"));
    setWindowIcon(QIcon(":/images/icon.png"));
    QVBoxLayout *windowLayout = new QVBoxLayout(this);

    m_infoMessage->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_infoMessage->setMinimumWidth(750);
    m_infoMessage->setAlignment(Qt::AlignHCenter);

    windowLayout->addWidget(m_infoMessage);

    // Spectrograph and level meter

    QScopedPointer<QHBoxLayout> analysisLayout(new QHBoxLayout);
    analysisLayout->addWidget(m_spectrograph);
    windowLayout->addLayout(analysisLayout.data());
    analysisLayout.take();

    QScopedPointer<QHBoxLayout> infoLayout(new QHBoxLayout);
    m_silence->setStyleSheet("font-weight: bold; color: red");
    m_silence->setAlignment(Qt::AlignRight);
    infoLayout->addWidget(m_basicFrequencyInfoMessage);
    infoLayout->addWidget(m_silence);
    windowLayout->addLayout(infoLayout.data());
    infoLayout.take();

    // Button panel

    const QSize buttonSize(30, 30);

    m_recordIcon = QIcon(":/images/record.png");
    m_recordButton->setIcon(m_recordIcon);
    m_recordButton->setEnabled(false);
    m_recordButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_recordButton->setMinimumSize(buttonSize);

    m_pauseIcon = QIcon(":/images/pause.png");
    m_pauseButton->setIcon(m_pauseIcon);
    m_pauseButton->setEnabled(false);
    m_pauseButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_pauseButton->setMinimumSize(buttonSize);

    m_playIcon = QIcon(":/images/play.png");
    m_playButton->setIcon(m_playIcon);
    m_playButton->setEnabled(false);
    m_playButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_playButton->setMinimumSize(buttonSize);

    m_settingsIcon = QIcon(":/images/settings.png");
    m_settingsButton->setIcon(m_settingsIcon);
    m_settingsButton->setEnabled(true);
    m_settingsButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_settingsButton->setMinimumSize(buttonSize);

    QScopedPointer<QHBoxLayout> buttonPanelLayout(new QHBoxLayout);
    buttonPanelLayout->addStretch();
    buttonPanelLayout->addWidget(m_recordButton);
    buttonPanelLayout->addWidget(m_pauseButton);
    buttonPanelLayout->addWidget(m_playButton);
    buttonPanelLayout->addWidget(m_settingsButton);

    QWidget *buttonPanel = new QWidget(this);
    buttonPanel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    buttonPanel->setLayout(buttonPanelLayout.data());
    buttonPanelLayout.take(); // ownership transferred to buttonPanel

    QScopedPointer<QHBoxLayout> bottomPaneLayout(new QHBoxLayout);
    bottomPaneLayout->addWidget(buttonPanel);
    windowLayout->addLayout(bottomPaneLayout.data());
    bottomPaneLayout.take(); // ownership transferred to windowLayout

    // Apply layout

    setLayout(windowLayout);
}

void MainWidget::connectUi()
{
    connect(m_recordButton, &QPushButton::clicked,
            m_engine, &Engine::startRecording);

    connect(m_pauseButton, &QPushButton::clicked,
            m_engine, &Engine::suspend);

    connect(m_playButton, &QPushButton::clicked,
            m_engine, &Engine::startPlayback);

    connect(m_settingsButton, &QPushButton::clicked,
            this, &MainWidget::showSettingsDialog);

    connect(m_engine, &Engine::stateChanged,
            this, &MainWidget::stateChanged);

    connect(m_engine, &Engine::dataLengthChanged,
            this, &MainWidget::updateButtonStates);

    connect(m_engine, &Engine::recordPositionChanged,
            this, &MainWidget::audioPositionChanged);

    connect(m_engine, &Engine::playPositionChanged,
            this, &MainWidget::audioPositionChanged);

    connect(m_engine, QOverload<qint64, qint64, const FrequencySpectrum&>::of(&Engine::spectrumChanged),
            this, QOverload<qint64, qint64, const FrequencySpectrum&>::of(&MainWidget::spectrumChanged));

    connect(m_engine, &Engine::infoMessage,
            this, &MainWidget::infoMessage);

    connect(m_engine, &Engine::errorMessage,
            this, &MainWidget::errorMessage);

    connect(m_engine, &Engine::displaySilenceLabel,
            this, &MainWidget::displaySilenceLabel);

    connect(m_engine, &Engine::baseFrequencyChanged,
            this, &MainWidget::baseFrequencyChanged);

    connect(m_spectrograph, &Spectrograph::infoMessage,
            this, &MainWidget::infoMessage);

    if (m_engine->initializeRecord())
       updateButtonStates();
}

void MainWidget::updateButtonStates()
{
    const bool recordEnabled = ((QAudio::AudioOutput == m_engine->mode() ||
                                (QAudio::ActiveState != m_engine->state() &&
                                 QAudio::IdleState != m_engine->state())));
    m_recordButton->setEnabled(recordEnabled);

    const bool pauseEnabled = (QAudio::ActiveState == m_engine->state() ||
                               QAudio::IdleState == m_engine->state());
    m_pauseButton->setEnabled(pauseEnabled);

    const bool playEnabled = ((QAudio::AudioOutput != m_engine->mode() ||
                               (QAudio::ActiveState != m_engine->state() &&
                                QAudio::IdleState != m_engine->state() &&
                                QAudio::InterruptedState != m_engine->state())));
    m_playButton->setEnabled(playEnabled);
}

void MainWidget::reset()
{
    m_engine->reset();
    m_spectrograph->reset();
}
