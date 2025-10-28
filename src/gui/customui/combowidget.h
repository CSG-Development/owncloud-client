#pragma once

#include <QFrame>

class QLineEdit;
class QLabel;
class QToolButton;
class QComboBox;

namespace Ui {class ComboWidget;}

//class PopupComboWidget;

class ComboWidget: public QFrame
{
    Q_OBJECT

public:
    explicit ComboWidget(QWidget* parent = nullptr);
    ~ComboWidget();

    QComboBox* comboBox() const;

    void setFontPixelSize(int val);
    int fontPixelSize() const;

    void setPlaceholderText(const QString& str);

    QString text() const;
    void setText(const QString& val);

    void setErrorState(bool enable, const QString& txt = QString());

    bool eventFilter(QObject *watched, QEvent *event) override;

public slots:
    void setDarkTheme();

Q_SIGNALS:
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
    Ui::ComboWidget* ui = nullptr;

    QLabel* promptLabel = nullptr;
    //PopupComboWidget* popup = nullptr;

    bool isDark = false;
    bool errorState = false;
};
