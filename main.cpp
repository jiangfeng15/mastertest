#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QMessageBox::information(NULL, "TEST", "测试窗口",QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);

    return a.exec();
}
