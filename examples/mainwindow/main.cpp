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

    // MainWindow w;
    // w.show();

    MessageBox msgBox1;
    msgBox1.setIcon(QMessageBox::Warning);
    msgBox1.setText("This is a warning message");
    msgBox1.setInformativeText("This is additional information");
    msgBox1.setWindowTitle("Warning");
    msgBox1.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox1.exec();

    return a.exec();
}