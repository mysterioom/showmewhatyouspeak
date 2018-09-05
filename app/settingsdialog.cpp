#include "settingsdialog.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(
            const QList<QAudioDeviceInfo> &availableInputDevices,
            const QList<QAudioDeviceInfo> &availableOutputDevices,
            const int &thresholdSilence,
            QWidget *parent)
    :   QDialog(parent)
    ,   m_inputDeviceComboBox(new QComboBox(this))
    ,   m_outputDeviceComboBox(new QComboBox(this))
    ,   m_thresholdOfSilenceSlider(new QSlider(Qt::Horizontal, this))
    ,   m_thresholdOfSilenceValueLabel(new QLabel(this))
{
    QVBoxLayout *dialogLayout = new QVBoxLayout(this);
    Q_UNUSED(thresholdSilence);

    QAudioDeviceInfo device;
    foreach (device, availableInputDevices)
        m_inputDeviceComboBox->addItem(device.deviceName(),
                                       QVariant::fromValue(device));
    foreach (device, availableOutputDevices)
        m_outputDeviceComboBox->addItem(device.deviceName(),
                                       QVariant::fromValue(device));

    m_thresholdOfSilenceSlider->setFocusPolicy(Qt::StrongFocus);
    m_thresholdOfSilenceSlider->setTickPosition(QSlider::TicksBothSides);
    m_thresholdOfSilenceSlider->setTickInterval(10);
    m_thresholdOfSilenceSlider->setSingleStep(10);
    m_thresholdOfSilenceSlider->setMinimum(minimumThresholdOfSilence());
    m_thresholdOfSilenceSlider->setMaximum(maximumThresholdOfSilence());
    m_thresholdOfSilenceSlider->setValue(initThresholdOfSilence());

    m_thresholdOfSilenceValueLabel->setText(QString("%1 dB").arg(initThresholdOfSilence()));
    m_thresholdOfSilenceValueLabel->setAlignment(Qt::AlignCenter);

    if (!availableInputDevices.empty())
        m_inputDevice = availableInputDevices.front();
    if (!availableOutputDevices.empty())
        m_outputDevice = availableOutputDevices.front();

    QScopedPointer<QHBoxLayout> inputDeviceLayout(new QHBoxLayout);
    QLabel *inputDeviceLabel = new QLabel(tr("Input device"), this);
    inputDeviceLayout->addWidget(inputDeviceLabel);
    inputDeviceLayout->addWidget(m_inputDeviceComboBox);
    dialogLayout->addLayout(inputDeviceLayout.data());
    inputDeviceLayout.take();

    QScopedPointer<QHBoxLayout> outputDeviceLayout(new QHBoxLayout);
    QLabel *outputDeviceLabel = new QLabel(tr("Output device"), this);
    outputDeviceLayout->addWidget(outputDeviceLabel);
    outputDeviceLayout->addWidget(m_outputDeviceComboBox);
    dialogLayout->addLayout(outputDeviceLayout.data());
    outputDeviceLayout.take();

    QScopedPointer<QHBoxLayout> thresholdOfSilenceLayout(new QHBoxLayout);
    QLabel *thresholdOfSilenceLabel = new QLabel(tr("Threshold of silence"), this);
    thresholdOfSilenceLayout->addWidget(thresholdOfSilenceLabel);
    thresholdOfSilenceLayout->addWidget(m_thresholdOfSilenceSlider);
    dialogLayout->addLayout(thresholdOfSilenceLayout.data());
    thresholdOfSilenceLayout.take();

    QScopedPointer<QHBoxLayout> thresholdOfSilenceValueLayout(new QHBoxLayout);
    thresholdOfSilenceValueLayout->addWidget(m_thresholdOfSilenceValueLabel);
    dialogLayout->addLayout(thresholdOfSilenceValueLayout.data());
    thresholdOfSilenceValueLayout.take();

    connect(m_inputDeviceComboBox, QOverload<int>::of(&QComboBox::activated),
            this, &SettingsDialog::inputDeviceChanged);
    connect(m_outputDeviceComboBox, QOverload<int>::of(&QComboBox::activated),
            this, &SettingsDialog::outputDeviceChanged);
    connect(m_thresholdOfSilenceSlider, SIGNAL(valueChanged(int)), this, SLOT(thresholdSilenceChanged(int)));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    dialogLayout->addWidget(buttonBox);

    connect(buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked,
            this, &SettingsDialog::accept);
    connect(buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked,
            this, &SettingsDialog::reject);

    setLayout(dialogLayout);
}

SettingsDialog::~SettingsDialog()
{

}

void SettingsDialog::inputDeviceChanged(int index)
{
    m_inputDevice = m_inputDeviceComboBox->itemData(index).value<QAudioDeviceInfo>();
}

void SettingsDialog::outputDeviceChanged(int index)
{
    m_outputDevice = m_outputDeviceComboBox->itemData(index).value<QAudioDeviceInfo>();
}

void SettingsDialog::thresholdSilenceChanged(int value)
{
    m_thresholdOfSilenceValueLabel->setText(QString("%1 dB").arg(value));
    m_thresholdSilence = value;
}
