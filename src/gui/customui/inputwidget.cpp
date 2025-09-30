#include "inputwidget.h"
#include "stylehelper.h"
#include "theme.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QLabel>

namespace {
constexpr int promptFontSize = 12;
constexpr int btnIconSize = 21;
constexpr int horizMargin = 16;
constexpr int vertMargin = 4;
constexpr int btnSpacing = 7;
constexpr int editFrameHeight = 56;

constexpr qreal frameWidth = 1;
constexpr qreal frameRadius = 20;
constexpr int fontPixelSize = 16;

QPair<QColor,QColor> frameColor = {
    QColor("#CBCDD3"),
    QColor("#616161")
};
QPair<QString,QString> inputStyle = {
    QStringLiteral(":/res/inputwidget/inputwidget_light.qss"),
    QStringLiteral(":/res/inputwidget/inputwidget_dark.qss")
};
}

InputWidget::InputWidget(QWidget *parent)
    : QFrame(parent)
{
    frame_ = new QFrame(this);
    frame_->setObjectName(QStringLiteral("frame"));
    frame_->setMinimumHeight(editFrameHeight);
    frame_->setMaximumHeight(editFrameHeight);

    frame_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QHBoxLayout* l = new QHBoxLayout;
    l->setContentsMargins(horizMargin, vertMargin, horizMargin, vertMargin);
    l->setSpacing(btnSpacing);
    frame_->setLayout(l);

    lineEdit_ = new QLineEdit(this);
    lineEdit_->setObjectName(QStringLiteral("lineEdit"));
    lineEdit_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    l->addWidget(lineEdit_);

    prompt_ = new QLabel(this);
    prompt_->setObjectName(QStringLiteral("promptText"));
    QFont f = prompt_->font();
    f.setPixelSize(promptFontSize);
    prompt_->setFont(f);
    prompt_->setAutoFillBackground(true);
    prompt_->setVisible(false);
    updateSize();

    showPassButton_ = new QToolButton(this);
    showPassButton_->setObjectName(QStringLiteral("btnPassword"));
    showPassButton_->setVisible(false);
    showPassButton_->setCheckable(true);
    showPassButton_->setCursor(Qt::PointingHandCursor);
    showPassButton_->setIconSize(QSize(btnIconSize, btnIconSize));
    l->addWidget(showPassButton_);

    connect(showPassButton_, &QToolButton::clicked, this, [this] {
        lineEdit_->setEchoMode(showPassButton_->isChecked() ? QLineEdit::Normal : QLineEdit::Password);
    });
    connect(lineEdit_, &QLineEdit::textChanged, this, &InputWidget::onTextChanged);

    setDarkTheme();
}

QSize InputWidget::minimumSizeHint() const
{
    int sz = editFrameHeight + promptFontSize / 2;
    return {sz, sz};
}

int InputWidget::fontPixelSize() const
{
    return lineEdit_->font().pixelSize();
}

void InputWidget::setPasswordMode(bool val)
{
    lineEdit_->setEchoMode(val ? QLineEdit::Password : QLineEdit::Normal);
    const QSignalBlocker b_(showPassButton_);
    showPassButton_->setChecked(!val);
    showPassButton_->setVisible(val);
}

void InputWidget::setPasswordButtonImage(const QString &val)
{
    showPassButton_->setIcon(QIcon(val));
}

void InputWidget::setPlaceholderText(const QString &str)
{
    lineEdit_->setPlaceholderText(str);
    prompt_->setText(str);
}

QString InputWidget::text() const {return lineEdit_->text();}

void InputWidget::setText(const QString &val)
{
    lineEdit_->setText(val);
}

void InputWidget::setDarkTheme()
{
    isDark = CUR::Theme::instance()->isDarkTheme();
    updateStyles();
}

void InputWidget::setFontPixelSize(int val)
{
    auto font = lineEdit_->font();
    font.setPixelSize(val);
    lineEdit_->setFont(font);
}

void InputWidget::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);
}

void InputWidget::resizeEvent(QResizeEvent */*event*/)
{
    updateSize();
}

void InputWidget::onTextChanged(const QString& str)
{
    if (!prompt_->text().isEmpty())
        prompt_->setVisible(!str.isEmpty());
    Q_EMIT textChanged(str);
}

void InputWidget::updateSize()
{
    frame_->resize(rect().width() - 1, rect().height() - (promptFontSize / 2));
    frame_->move(rect().x() + 1, rect().y() + (promptFontSize / 2));
    prompt_->move(rect().left() + 16, frame_->rect().top() - 2);
}

void InputWidget::updateStyles()
{
    setStyleSheet(CUR::StyleHelper::loadFileToString(isDark ? inputStyle.second : inputStyle.first));
    update();
}
