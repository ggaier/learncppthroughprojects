#include "textfinder.h"
#include "ui_textfinder.h"

#include <QFile>
#include <QTextStream>

TextFinder::TextFinder(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TextFinder)
{
    ui->setupUi(this);
    loadTextFile();
}

TextFinder::~TextFinder()
{
    delete ui;
}

void TextFinder::loadTextFile(){
    //QFile用来读写文件
    QFile inputFile(":/input.txt");
    //enum type和其中的enumerator被放到同一个命名空间中.
    //一旦使用了enum class就不会有这个问题了.
    inputFile.open(QIODevice::ReadOnly);

    //QTextStream 对QIODevice, QByteArray, QString进行操作, 可以方便的读取和写入文字.
    QTextStream in(&inputFile);
    //读取stream中全部的内容, 并返回一个QString
    QString line = in.readAll();
    inputFile.close();

    ui->textEdit->setPlainText(line);
    QTextCursor cursor = ui->textEdit->textCursor();
    cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor, 1);



}

void TextFinder::on_pushButton_clicked()
{
    QString searchString = ui->lineEdit->text();
    ui->textEdit->find(searchString, QTextDocument::FindWholeWords);
}

