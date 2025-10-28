#pragma once

#include <QDialog>

namespace Ui { class CodeDialog; }

class CodeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CodeDialog(QWidget *parent = nullptr);
    ~CodeDialog();

    void updateTheme();

    QString getCode() const;

private:
    Ui::CodeDialog *ui = nullptr;
};
