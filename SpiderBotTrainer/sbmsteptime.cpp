#include "sbmsteptime.h"

#include <QHBoxLayout>
#include <QDebug>
#include <math.h>

SbmStepTime::SbmStepTime(double timeInterval, uint32_t timeInSteps, QWidget *parent) :
        QWidget(parent),
        m_timeInSteps(timeInSteps),
        m_timeInterval(timeInterval),
        sbVal(new QDoubleSpinBox()),
        bUnits(new QToolButton()) {
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setSpacing(2);
    //layout->setMargin(0);

    layout->addWidget(sbVal);

    bUnits->setText("ms");
    bUnits->setCheckable(true);
    bUnits->setChecked(true);
    connect(bUnits, &QToolButton::clicked, this, &SbmStepTime::on_bUnits_clicked);
    layout->addWidget(bUnits);

    setTimeInterval(timeInterval);
    connect(sbVal, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &SbmStepTime::on_sbVal_valueChanged);
    connect(sbVal, &QDoubleSpinBox::editingFinished, this, &SbmStepTime::on_sbVal_editingFinished);

    setLayout(layout);
}

uint32_t SbmStepTime::getTimeInSteps() const {
    return m_timeInSteps;
}

double SbmStepTime::getTimeInterval() const {
    return m_timeInterval;
}

void SbmStepTime::setTimeInSteps(uint32_t timeInSteps) {
    m_timeInSteps = timeInSteps;
    disconnect(sbVal, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &SbmStepTime::on_sbVal_valueChanged);
    if (bUnits->isChecked()) { // ms
        sbVal->setValue(m_timeInSteps * m_timeInterval);
    } else { // steps
        sbVal->setValue(m_timeInSteps);
    }
    connect(sbVal, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &SbmStepTime::on_sbVal_valueChanged);
}

void SbmStepTime::setTimeInterval(double timeInterval) {
    double time = m_timeInSteps * m_timeInterval;
    m_timeInSteps = round(time / timeInterval);
    if (bUnits->isChecked()) { // ms
        sbVal->setSingleStep(timeInterval);
        sbVal->setMinimum(timeInterval);
        sbVal->setMaximum(100 * timeInterval);
        sbVal->setValue(m_timeInSteps * timeInterval);
    } else { // steps
        sbVal->setValue(m_timeInSteps);
    }
    m_timeInterval = timeInterval;
}

void SbmStepTime::on_bUnits_clicked(bool checked) {
    disconnect(sbVal, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &SbmStepTime::on_sbVal_valueChanged);
    if (checked) { // ms
        sbVal->setSuffix(" ms");
        sbVal->setDecimals(3);
        sbVal->setSingleStep(m_timeInterval);
        sbVal->setMinimum(m_timeInterval);
        sbVal->setMaximum(100 * m_timeInterval);
        sbVal->setValue(m_timeInSteps * m_timeInterval);
    } else { // steps
        sbVal->setSuffix(" steps");
        sbVal->setDecimals(0);
        sbVal->setSingleStep(1);
        sbVal->setMinimum(1);
        sbVal->setMaximum(100);
        sbVal->setValue(m_timeInSteps);
    }
    connect(sbVal, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &SbmStepTime::on_sbVal_valueChanged);
}

void SbmStepTime::on_sbVal_editingFinished() {
    if (bUnits->isChecked()) { // ms
        sbVal->setValue(round(sbVal->value() / m_timeInterval) * m_timeInterval);
    }
}

void SbmStepTime::on_sbVal_valueChanged(double val) {
    if (bUnits->isChecked()) { // ms
        m_timeInSteps = static_cast<uint32_t>(round(static_cast<double>(val) / m_timeInterval));
    } else {
        m_timeInSteps = static_cast<uint32_t>(val);
    }
    emit onChangedValue(m_timeInSteps, m_timeInterval);
}
