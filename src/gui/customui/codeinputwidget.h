#pragma once

#include <QWidget>

namespace Ui { class CodeInputWidget; }

class QLineEdit;

class CodeInputWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CodeInputWidget(QWidget *parent = nullptr);
    ~CodeInputWidget();

    QString codeStr() const;
    void clearCode();

    void setErrorState(bool enable);

signals:
    void codeChanged();

public slots:
    void setDarkTheme();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
    void updateStyles();

private:
    void onTextEdited(const QString& txt);
    void moveFocus(bool forward);
    int indexOfEdit(QWidget* ed);
    bool validateClipboardBuffer();
    void pasteCode();
    void onContextMenuRequested(const QPoint &pos);

private:
    Ui::CodeInputWidget* ui = nullptr;
    QList<QLineEdit*> edPtrs;
    QAction* pasteCodeAction = nullptr;
    bool isDark = false;
    bool errorState = false;
};
