#pragma once

#include <QFrame>

class QLineEdit;
class QLabel;
class QToolButton;

namespace Ui {class InputWidget;}


class InputWidget: public QFrame
{
    Q_OBJECT

public:
    explicit InputWidget(QWidget* parent = nullptr);
    ~InputWidget();

    QLineEdit* lineEdit() const;

    void setFontPixelSize(int val);
    int fontPixelSize() const;

    void setPasswordMode(bool val);
    void setPasswordButtonImage(const QString& val);

    void setPlaceholderText(const QString& str);

    QString text() const;
    void setText(const QString& val);

    void setErrorState(bool enable, const QString& txt = QString());

    bool eventFilter(QObject *watched, QEvent *event) override;

public slots:
    void setDarkTheme();

Q_SIGNALS:
    void frameColorChanged();
    void frameWidthChanged();
    void frameRadiusChanged();
    void fontPixelSizeChanged();

    void textChanged(const QString &);
    void textEdited(const QString &);
    void editingFinished();
    void focusReceived();
    void focusLost();

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

    bool isDark = false;
    bool errorState = false;
};
