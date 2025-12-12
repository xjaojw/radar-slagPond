/********************************************************************************
** Form generated from reading UI file 'SlagPondWidget.ui'
**
** Created by: Qt User Interface Compiler version 6.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SLAGPONDWIDGET_H
#define UI_SLAGPONDWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SlagPondWidget
{
public:

    void setupUi(QWidget *SlagPondWidget)
    {
        if (SlagPondWidget->objectName().isEmpty())
            SlagPondWidget->setObjectName("SlagPondWidget");
        SlagPondWidget->resize(800, 600);

        retranslateUi(SlagPondWidget);

        QMetaObject::connectSlotsByName(SlagPondWidget);
    } // setupUi

    void retranslateUi(QWidget *SlagPondWidget)
    {
        SlagPondWidget->setWindowTitle(QCoreApplication::translate("SlagPondWidget", "SlagPondWidget", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SlagPondWidget: public Ui_SlagPondWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SLAGPONDWIDGET_H
