#pragma once

#include <QDialog>

class CustomInputDlgPrivate;

namespace APP {

class CustomInputDlg: public QObject
{
    Q_OBJECT

public:
    explicit CustomInputDlg(QWidget *parent = nullptr);
    ~CustomInputDlg();

    int exec();

    CustomInputDlg& setHeaderText(const QString& headerText);
    CustomInputDlg& setPromptText(const QString& promptText);
    CustomInputDlg& setAcceptButtonText(const QString& acceptText);
    CustomInputDlg& setRejectButtonText(const QString& rejectText);

    QString inputText() const;

    QWidget* widgetPtr() const;

public slots:
    void accept();
    void reject();

signals:
    void accepted();
    void rejected();
    void finished(int result);

private:
    Q_DECLARE_PRIVATE(CustomInputDlg)
    QScopedPointer<CustomInputDlgPrivate> d_ptr;
};

} // namespace APP
