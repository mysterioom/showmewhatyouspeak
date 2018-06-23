#ifndef FFT_H
#define FFT_H

#include <QObject>
#include <vector>
#include <memory>

using std::vector;
using std::shared_ptr;

class FFT : public QObject
{
    Q_OBJECT
public:
    explicit FFT(QObject *parent = 0);
    ~FFT();

private:
    uint fftSize = 2048; // Default FFT size
    double* array = nullptr;
    int* ip = nullptr;
    double* w = nullptr;

    void clear();


public slots:
    void initialize();
    void setFFTSize(uint fftSize);
    void signalData(shared_ptr<vector<vector<float>>> data);

signals:
    void calculated(shared_ptr<vector<vector<double>>> data);
    void error(QString error);
};

#endif
