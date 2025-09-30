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
    
    static QIcon getIcon(const QString& name, bool isDark);

    static QIcon getDotsIcon(const QStyleOptionToolButton* opt);
    static QIcon getArrowIcon(Qt::ArrowType arrow, bool isPressed, bool isDisabled, bool isDark);

    static void setDarkMode(bool dark);

    static QProxyStyle* toolbarMenuStyle() {return tbMenuStyle_;}
    static QProxyStyle* pushButtonStyle() {return pushButtonStyle_;}

    static StyleHelper* getInstance();

    static QString loadFileToString(const QString& fileName);

private:
    static StyleHelper* instance_;
    static QProxyStyle* tbMenuStyle_;
    static QProxyStyle* pushButtonStyle_;
};

} // namespace CUR
