#pragma once

#include <QWidget>
#include <QProperty>

namespace Ui { class CodeInputWidget; }

class QLineEdit;

class CodeInputWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool darkTheme READ isDarkTheme WRITE setDarkTheme BINDABLE bindableDarkTheme)

public:
    explicit CodeInputWidget(QWidget *parent = nullptr);
    ~CodeInputWidget();

    QString codeStr() const;
    void clearCode();
    void focusFirstCell();

    void setErrorState(bool enable);

    bool isDarkTheme() const { return darkTheme_.value(); }
    void setDarkTheme(bool v) { darkTheme_.setValue(v); }

    QBindable<bool> bindableDarkTheme() {return &darkTheme_;}

signals:
    void codeChanged();
    void focusGained();

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

    QString buildCode() const;

private:
    Ui::CodeInputWidget* ui = nullptr;
    QList<QLineEdit*> edPtrs;
    QAction* pasteCodeAction = nullptr;
    bool errorState = false;
    QString code_;

    QProperty<bool> darkTheme_ {false};
    QPropertyNotifier themeNotifier;
};
