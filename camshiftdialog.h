#ifndef CAMSHIFTDIALOG_H
#define CAMSHIFTDIALOG_H

#include <QDialog>
#include "ui_camshiftdialog.h"
#include "camshift.h"

class CamShiftDialog : public QDialog, public Ui::CamshiftDialog {
    Q_OBJECT
    public:
        CamShiftDialog(QWidget *parent = 0);
};

#endif // CAMSHIFTDIALOG_H
