#pragma once

#include <QFrame>

class QLineEdit;
class QLabel;

class InputWidget: public QFrame
{
    Q_OBJECT

public:
    explicit InputWidget(QWidget* parent = nullptr);

    QSize minimumSizeHint() const override;

    QLineEdit* lineEdit() const {return lineEdit_;}

    void setFontPixelSize(int val);
    int fontPixelSize() const;

    void setPasswordMode(bool val);
    void setPasswordButtonImage(const QString& val);

    void setPlaceholderText(const QString& str);

    QString text() const;
    void setText(const QString& val);

public slots:
    void setDarkTheme();

Q_SIGNALS:
    void frameColorChanged();
    void frameWidthChanged();
    void frameRadiusChanged();
    void fontPixelSizeChanged();

    void textChanged(const QString &);

    void buttonClicked();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    void onTextChanged(const QString&);
    void updateSize();
    void updateStyles();

private:
    QLineEdit* lineEdit_ = nullptr;
    QToolButton* showPassButton_ = nullptr;
    QLabel* prompt_ = nullptr;
    QFrame* frame_ = nullptr;

    bool isDark = false;
};
