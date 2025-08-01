#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>
#include <QtGui/QAccessibleEvent>

#include "mainwindow.h"

class MessageBox : public QMessageBox {
public:
    MessageBox(QWidget *parent = nullptr) : QMessageBox(parent) {
    }

    void mousePressEvent(QMouseEvent *event) override {
        QMessageBox::mousePressEvent(event);

        QAccessibleEvent e(this, QAccessible::Alert);
        QAccessible::updateAccessibility(&e);
    }
};

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    return a.exec();
}