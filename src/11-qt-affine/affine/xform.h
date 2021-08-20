#ifndef XFORM_H
#define XFORM_H

#include "arthurwidgets.h"

class HoverPoints;

QT_BEGIN_NAMESPACE
class QLineEdit;
QT_END_NAMESPACE

class XForm : public ArthurFrame
{
    Q_OBJECT

public:
    XForm(QWidget *parent = nullptr);
    ~XForm();
};
#endif // XFORM_H
