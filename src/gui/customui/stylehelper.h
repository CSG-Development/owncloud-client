#pragma once

#include <QObject>
#include <QIcon>
#include <QProxyStyle>

class QStyleOption;
class QStyleOptionToolButton;

namespace CUR {

class StyleHelper: public QObject
{
    Q_OBJECT

public:
    explicit StyleHelper(QObject* parent = nullptr);
    ~StyleHelper();

    static void applyPushButtonStyle(QWidget* root);

    static qreal dpi(const QStyleOption* option);
    static qreal dpiScaled(qreal value, qreal dpi);

    static QIcon getIcon(const QString& name);

    static QPixmap getDotsPixmap(const QStyleOptionToolButton* opt);
    static QPixmap getArrowPixmap(const QStyleOptionToolButton* opt, bool isDark);
    static QPixmap getArrowPixmap(Qt::ArrowType arrow, bool isPressed, bool isDisabled, bool isDark);

    static void setDarkMode(bool dark);

    static QProxyStyle* toolbarMenuStyle() {return tbMenuStyle_;}
    static QProxyStyle* pushButtonStyle() {return pushButtonStyle_;}

    static StyleHelper* getInstance();

private:
    static StyleHelper* instance_;
    static QProxyStyle* tbMenuStyle_;
    static QProxyStyle* pushButtonStyle_;
};

} // namespace CUR
