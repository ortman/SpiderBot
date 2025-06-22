#include "footitem.h"

#include <QDebug>
#include <QBoxLayout>
#include <QKeyEvent>

FootItem::FootItem(uint32_t segmentCount, float *angles, QWidget *parent):
        m_editVisible(false),
        m_readOnly(false),
        mainLayout(new QHBoxLayout()),
        lName(new QLabel()),
        lValues(new QLabel()),
        layoutEditor(new QHBoxLayout()) {
    Q_UNUSED(parent)
    this->segmentCount = segmentCount;
    this->angles = angles;

    //mainLayout->setMargin(3);
    mainLayout->setSpacing(0);
    this->setLayout(mainLayout);

    mainLayout->addWidget(lName, 0);
    mainLayout->addWidget(lValues, 1);

    //layoutEditor->setMargin(0);
    layoutEditor->setSpacing(0);
    QLabel *l;
    for (uint32_t i=0; i<segmentCount; ++i) {        
        if (i > 0) {
            l = new QLabel("°, ");
            l->setVisible(false);
            layoutEditor->addWidget(l);
        }
        layoutEditor->addWidget(createLineEditor(&angles[i]));
    }
    l = new QLabel("°)");
    l->setVisible(false);
    layoutEditor->addWidget(l);
    mainLayout->addLayout(layoutEditor, 1);

    updateLabelValues();
}

QSize FootItem::sizeHint() const {
    return QSize(100, 30);
}

void FootItem::updateLabelValues(void) {
    QString text = "";
    for (uint32_t i=0; i<segmentCount; ++i) {
        text.append(QString::number(angles[i])).append("°, ");
    }
    text.chop(2);
    text.append(')');
    lValues->setText(text);
}

QLineEdit *FootItem::createLineEditor(float *val) {
    QLineEdit *e = new QLineEdit();
    e->setVisible(false);
    e->setMinimumHeight(24);
    e->setText(QString::number(*val));
    connect(e, &QLineEdit::textEdited, this, [this, val](QString text){
        *val= text.toFloat();
        emit onFootAngleChanged(segmentCount, angles);
    });
    return e;
}

void FootItem::setFootName(QString footName) {
    lName->setText(footName.append(" ("));
}

void FootItem::setFootIndex(int footIndex) {
    lName->setText(QString("Foot ").append(QString::number(footIndex)).append(" ("));
}

void FootItem::setEditVisible(bool visible) {
    if (m_editVisible != visible) {
        if (visible) {
            lValues->setVisible(false);
            int cnt = layoutEditor->count();
            for (int i=0; i<cnt; i++) {
                layoutEditor->itemAt(i)->widget()->setVisible(true);
            }
        } else {
            int cnt = layoutEditor->count();
            for (int i=0; i<cnt; i++) {
                layoutEditor->itemAt(i)->widget()->setVisible(false);
            }
            updateLabelValues();
            lValues->setVisible(true);
        }
        m_editVisible = visible;
    }
}

bool FootItem::isEditVisible() const {
    return m_editVisible;
}

bool FootItem::isReadOnly() const {
    return m_readOnly;
}

void FootItem::mouseDoubleClickEvent(QMouseEvent *) {
    if (!m_readOnly) {
        setEditVisible(!m_editVisible);
    }
}

void FootItem::keyPressEvent(QKeyEvent *event) {
    int key = event->key();
    if (key == Qt::Key_Up || key == Qt::Key_Down) {
        QLineEdit *e = dynamic_cast<QLineEdit*>(focusWidget());
        if (e != nullptr) {
            float val = e->text().toFloat();
            if (key == Qt::Key_Up) {
                e->setText(QString::number(val+1));
                e->textEdited(e->text());
            }
            if (key == Qt::Key_Down) {
                e->setText(QString::number(val-1));
                e->textEdited(e->text());
            }
        }
    }
}

void FootItem::setReadOnly(bool readOnly) {
    m_readOnly = readOnly;
}
