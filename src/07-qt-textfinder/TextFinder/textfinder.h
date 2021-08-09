#ifndef TEXTFINDER_H
#define TEXTFINDER_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class TextFinder; }
QT_END_NAMESPACE

class TextFinder : public QWidget
{
    Q_OBJECT

public:
    TextFinder(QWidget *parent = nullptr);
    ~TextFinder();

private slots:
    void on_pushButton_clicked();

private:
    Ui::TextFinder *ui;
    void loadTextFile();
};
#endif // TEXTFINDER_H
