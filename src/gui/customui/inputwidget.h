#pragma once

#include <QFrame>
#include <QProperty>

class QLineEdit;
class QLabel;
class QToolButton;

namespace Ui {class InputWidget;}


class InputWidget: public QFrame
{
    Q_OBJECT
    Q_PROPERTY(bool darkTheme READ isDarkTheme WRITE setDarkTheme BINDABLE bindableDarkTheme)

public:
    explicit InputWidget(QWidget* parent = nullptr);
    ~InputWidget();

    QLineEdit* lineEdit() const;

    void setFontPixelSize(int val);
    int fontPixelSize() const;

    void setPasswordMode(bool val);
    void setPasswordButtonImage(const QString& val);

    void setReadOnly(bool ro);

    void setPlaceholderText(const QString& str);

    QString text() const;
    void setText(const QString& val);

    void setErrorState(bool enable, const QString& txt = QString());

    bool eventFilter(QObject *watched, QEvent *event) override;

    bool isDarkTheme() const { return darkTheme_.value(); }
    void setDarkTheme(bool v) { darkTheme_.setValue(v); }

    QBindable<bool> bindableDarkTheme() {return &darkTheme_;}

Q_SIGNALS:
    void frameColorChanged();
    void frameWidthChanged();
    void frameRadiusChanged();
    void fontPixelSizeChanged();

    void textChanged(const QString &);
    void textEdited(const QString &);
    void editingFinished();
    void focusChanged(bool focused);

    void buttonClicked();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    void onTextChanged(const QString&);
    void updatePromptPosition();
    void updateStyles();

private:
    Ui::InputWidget* ui = nullptr;
    QLabel* promptLabel = nullptr;

    bool errorState = false;
    QPropertyNotifier themeNotifier;
    QProperty<bool> darkTheme_ {false};
};
