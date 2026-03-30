#pragma once

#include <QWidget>
#include <QDialog>

class CustomMessageBoxPrivate;

namespace APP {

class CustomMessageBox: public QObject
{
    Q_OBJECT

public:
    explicit CustomMessageBox(QWidget *parent = nullptr);
    ~CustomMessageBox();

    int exec();
    void show();
    void open();


    CustomMessageBox& setHeaderText(const QString& headerText);
    CustomMessageBox& setMessageText(const QString& messageText);
    CustomMessageBox& setAcceptButtonText(const QString& acceptText);
    CustomMessageBox& setRejectButtonText(const QString& rejectText);
    CustomMessageBox& setWarningIconVisible(bool visible);
    CustomMessageBox& setWide(bool wide);
    CustomMessageBox& setDefaultButton(QDialog::DialogCode code);
    CustomMessageBox& setSingleButton(bool val);
    CustomMessageBox& setSingleButtonText(const QString& text);

    CustomMessageBox& setDeleteOnClose(bool on = true);

    QWidget* widgetPtr() const;

public slots:
    void accept();
    void reject();

signals:
    void accepted();
    void rejected();
    void finished(int result);

private:
    Q_DECLARE_PRIVATE(CustomMessageBox)
    QScopedPointer<CustomMessageBoxPrivate> d_ptr;
};

} // namespace APP
