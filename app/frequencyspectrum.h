#ifndef FREQUENCYSPECTRUM_H
#define FREQUENCYSPECTRUM_H

#include <QtCore/QVector>

class FrequencySpectrum {
public:
    FrequencySpectrum(int numPoints = 0);

    struct Element {
        Element()
        :   frequency(0.0), amplitude(0.0), phase(0.0), clipped(false)
        { }

        qreal frequency; // in Hz
        qreal amplitude; // in range [0.0, 1.0]
        qreal phase; //in range [0.0, 2*PI]
        bool clipped; //whether value has been clipped during spectrum analysis
    };

    typedef QVector<Element>::iterator iterator;
    typedef QVector<Element>::const_iterator const_iterator;

    void reset();
    Element& operator[](int index);
    const Element& operator[](int index) const;
    iterator begin();
    iterator end();

private:
    QVector<Element> m_elements;

};

#endif // FREQUENCYSPECTRUM_H
