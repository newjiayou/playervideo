/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.10.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QSlider>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QWidget>
#include "videowidget.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    VideoWidget *screenwidget;
    QSlider *sliderProgress;
    QToolButton *btnPause;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(800, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        screenwidget = new VideoWidget(centralwidget);
        screenwidget->setObjectName("screenwidget");
        screenwidget->setGeometry(QRect(19, 9, 691, 401));
        sliderProgress = new QSlider(centralwidget);
        sliderProgress->setObjectName("sliderProgress");
        sliderProgress->setEnabled(false);
        sliderProgress->setGeometry(QRect(30, 430, 651, 20));
        sliderProgress->setStyleSheet(QString::fromUtf8("/* 1. \346\200\273\350\275\250\351\201\223 (\350\203\214\345\220\216\347\232\204\347\201\260\350\211\262\346\235\241) */\n"
"QSlider::groove:horizontal {\n"
"    border: 0px solid #999999;\n"
"    height: 4px; /* \350\275\250\351\201\223\345\216\232\345\272\246\357\274\214\350\266\212\345\260\217\350\266\212\347\262\276\350\207\264 */\n"
"    background: #E0E0E0; /* \346\265\205\347\201\260\350\211\262\350\203\214\346\231\257 */\n"
"    border-radius: 2px; /* \345\234\206\350\247\222 */\n"
"}\n"
"\n"
"/* 2. \346\273\221\345\235\227 (\351\202\243\344\270\252\345\234\206\347\220\203) */\n"
"QSlider::handle:horizontal {\n"
"    background: #FFFFFF; /* \347\272\257\347\231\275\347\220\203 */\n"
"    border: 2px solid #409EFF; /* \350\223\235\350\211\262\350\276\271\346\241\206 */\n"
"    width: 14px; /* \347\220\203\347\232\204\345\244\247\345\260\217 */\n"
"    height: 14px;\n"
"    border-radius: 9px; /* \345\215\212\345\276\204\357\274\214\345\217\230\346\210\220\346\255\243\345\234\206 */\n"
"    \n"
"    /* \343"
                        "\200\220\345\205\263\351\224\256\343\200\221\350\256\251\347\220\203\345\236\202\347\233\264\345\261\205\344\270\255 */\n"
"    /* \350\256\241\347\256\227\345\205\254\345\274\217\357\274\232(\350\275\250\351\201\223\351\253\230\345\272\2464 - \346\273\221\345\235\227\351\253\230\345\272\24614) / 2 = -5 */\n"
"    margin: -5px 0; \n"
"}\n"
"\n"
"/* 3. \345\267\262\346\222\255\346\224\276\347\232\204\351\203\250\345\210\206 (\350\223\235\350\211\262\346\235\241) */\n"
"QSlider::sub-page:horizontal {\n"
"    background: #409EFF; /* \350\277\231\351\207\214\346\215\242\346\210\220\344\275\240\345\226\234\346\254\242\347\232\204\351\242\234\350\211\262 */\n"
"    height: 4px;\n"
"    border-radius: 2px;\n"
"}\n"
"\n"
"/* 4. \346\234\252\346\222\255\346\224\276\347\232\204\351\203\250\345\210\206 (\351\200\232\345\270\270\345\222\214\350\275\250\351\201\223\344\270\200\346\240\267) */\n"
"QSlider::add-page:horizontal {\n"
"    background: #E0E0E0;\n"
"    height: 4px;\n"
"    border-radius: 2px;\n"
"}"));
        sliderProgress->setOrientation(Qt::Orientation::Horizontal);
        btnPause = new QToolButton(centralwidget);
        btnPause->setObjectName("btnPause");
        btnPause->setGeometry(QRect(270, 460, 80, 80));
        btnPause->setMinimumSize(QSize(50, 50));
        btnPause->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    border: none;\n"
"    border-radius: 40px; \n"
"    background-color: transparent; \n"
"}"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/src/true.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btnPause->setIcon(icon);
        btnPause->setIconSize(QSize(50, 50));
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 800, 20));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        btnPause->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
