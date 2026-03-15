#pragma once

#include <QString>
#include <QFile>
#include <QHash>
#include <QWidget>
#include <QStyle>
#include <QGraphicsDropShadowEffect>

namespace DlgUtils {

inline QString loadFileToString(const QString& fileName)
{
    // Static cache 'filePath' -> 'content'
    static QHash<QString, QString> styleCache;

    if (styleCache.contains(fileName)) {
        return styleCache.value(fileName);
    }

    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        QString content = QString::fromUtf8(file.readAll());
        styleCache.insert(fileName, content);
        return content;
    }

    return {};
}

inline void setTransparent(QWidget *target)
{
    if (!target) return;
#ifdef Q_OS_MACOS
    target->setWindowFlags(Qt::Window|Qt::FramelessWindowHint|Qt::NoDropShadowWindowHint|Qt::BypassWindowManagerHint);
    target->setAttribute(Qt::WA_NoSystemBackground, true);
#else
    target->setWindowFlags(Qt::Dialog|Qt::FramelessWindowHint|Qt::NoDropShadowWindowHint);
#endif
    target->setAttribute(Qt::WA_TranslucentBackground, true);
}

inline void applyDropShadowDialog(QWidget* target)
{
    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect(target);
#ifdef Q_OS_MACOS
    effect->setColor(QColor(0, 0, 0, 100));
    effect->setBlurRadius(20);
    effect->setXOffset(0);
    effect->setYOffset(0);
#else
    effect->setColor(QColor(0, 0, 0, 100));
    effect->setBlurRadius(20);
    effect->setXOffset(0);
    effect->setYOffset(0);
    // effect->setColor(QColor(0, 0, 0, 50));
    // effect->setBlurRadius(3);
    // effect->setXOffset(1);
    // effect->setYOffset(4);
#endif
    target->setGraphicsEffect(effect);
}

inline void setTheme(QWidget *target, bool isDark)
{
    if (!target) return;

    target->setProperty("theme", isDark ? QStringLiteral("dark") : QStringLiteral("light"));

    target->style()->unpolish(target);
    target->style()->polish(target);

    for (auto child : target->findChildren<QWidget*>()) {
        child->style()->unpolish(child);
        child->style()->polish(child);
    }
}

inline void clearStyleSheet(QWidget* target)
{
    if (!target) return;

    const auto children = target->findChildren<QWidget*>();
    for (QWidget* w : children) {
        w->setStyleSheet(QStringLiteral(""));
    }
}

inline void centerDialog(QWidget* parent, QWidget* dialog)
{
    if (!parent || !dialog) return;
    dialog->adjustSize();

    QRect parentRect = parent->frameGeometry();

    int x = parentRect.x() + (parentRect.width() - dialog->width()) / 2;
    int y = parentRect.y() + (parentRect.height() - dialog->height()) / 2;

    dialog->move(x, y);
}


}
