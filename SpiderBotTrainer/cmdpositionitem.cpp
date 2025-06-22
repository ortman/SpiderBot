#include "cmdpositionitem.h"
#include <QHBoxLayout>
#include <QToolButton>
#include <QStyle>
#include <QDebug>

CmdPositionItem::CmdPositionItem(sbmFootStepInfo_t *info, QWidget *parent) :
        QWidget(parent),
        m_readOnly(false),
        m_removable(true),
        lName(new QLabel) {
    this->info = info;
    createUI();
}

bool CmdPositionItem::isReadOnly() const {
    return m_readOnly;
}

bool CmdPositionItem::isRemovable() const {
    return m_removable;
}

void CmdPositionItem::setName(QString name) {
    lName->setText(name);
}

sbmFootStepInfo_t *CmdPositionItem::getStepInfo() {
    return info;
}

void CmdPositionItem::createUI() {
    QHBoxLayout *l = new QHBoxLayout();
    //l->setMargin(0);
    l->setSpacing(3);
    lName->setText("default position");
    lName->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

//    bRemove = new QToolButton();
//    bRemove->setAutoRaise(true);
//    bRemove->setToolTip("Remove");
//    bRemove->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
//    bRemove->setCursor(Qt::PointingHandCursor);

//    lName->setMinimumHeight(bRemove->sizeHint().height());
    lName->setMinimumHeight(20);

    l->addWidget(lName);
//    l->addWidget(bRemove);

    setLayout(l);
//    bRemove->setVisible(m_removable);
}

void CmdPositionItem::setReadOnly(bool readOnly) {
    m_readOnly = readOnly;
}

void CmdPositionItem::setRemovable(bool removable) {
    m_removable = removable;
//    bRemove->setVisible(removable);
}
