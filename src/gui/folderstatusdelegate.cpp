/*
 * Copyright (C) by Klaas Freitag <freitag@kde.org>
 * Copyright (C) by Olivier Goffart <ogoffart@woboq.com>
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#include "folderstatusdelegate.h"
#include "folderstatusmodel.h"

#include "customui/tool_button_dots.h"
#include "resources/resources.h"
#include "theme.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>

namespace {
const int barHeightC = 7;
// Status overlay icon size as a fraction of iconRect side length.
const qreal statusOverlaySizeRatioC = 0.5;
// Bottom inset from iconRect.bottom() as a fraction of iconRect height.
// Positive moves the overlay bottom up; negative moves it below iconRect.bottom().
const qreal statusOverlayBottomInsetRatioC = 0.1;
const QSizeF toolbutton_size{22, 22};
}

namespace APP {


FolderStatusDelegate::FolderStatusDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

// allocate each item size in listview.
QSize FolderStatusDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    updateFont(option.font);
    QFontMetricsF fm(_font);

    const auto classif = index.siblingAtColumn(static_cast<int>(FolderStatusModel::Columns::ItemType)).data().value<FolderStatusModel::ItemType>();
    if (classif != FolderStatusModel::RootFolder) {
        return QStyledItemDelegate::sizeHint(option, index);
    }

    // calc height
    qreal h = rootFolderHeightWithoutErrors() + _margin;
    // this already includes the bottom margin

    // add some space for the message boxes.
    for (auto column : {FolderStatusModel::Columns::FolderConflictMsg, FolderStatusModel::Columns::FolderErrorMsg, FolderStatusModel::Columns::FolderInfoMsg,
             FolderStatusModel::Columns::FolderWarningMsg}) {
        auto msgs = index.siblingAtColumn(static_cast<int>(column)).data().toStringList();
        if (!msgs.isEmpty()) {
            h += _margin + 2 * _margin + msgs.count() * fm.height();
        }
    }

    return QSize(0, h);
}

bool FolderStatusDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseMove || event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease) {
        auto *mouse_event = static_cast<QMouseEvent *>(event);

        const auto optionsButtonRect = this->computeOptionsButtonRect(option.rect);

        const bool oldHovered = (hoveredRow_ == index.row());
        const bool oldPressed = (pressedRow_ == index.row());

        if (event->type() == QEvent::MouseMove) {
            if (auto *w = const_cast<QWidget *>(option.widget)) {
                // Checkbox hover mouse cursor
                if (index.model()->flags(index) & Qt::ItemIsUserCheckable) {
                    QStyleOptionButton opt;
                    opt.QStyleOption::operator=(option);
                    opt.rect = option.rect;
                    if (auto *style = w->style())
                        checkboxRect = style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &opt, option.widget);
                } else {
                    checkboxRect = {};
                }
            }

            if (optionsButtonRect.contains(mouse_event->position())) {
                hoveredRow_ = index.row();
            } else {
                hoveredRow_ = -1;
            }
        }

        if (optionsButtonRect.contains(mouse_event->position())) {
            if (event->type() == QEvent::MouseButtonPress) {
                if (mouse_event->button() == Qt::LeftButton)
                    pressedRow_ = index.row();
            }
        }
        if (event->type() == QEvent::MouseButtonRelease) {
            if (mouse_event->button() == Qt::LeftButton)
                pressedRow_ = -1;
        }

        const bool newHovered = (hoveredRow_ == index.row());
        const bool newPressed = (pressedRow_ == index.row());
        if ((newHovered != oldHovered || newPressed != oldPressed) && option.widget)
            const_cast<QWidget *>(option.widget)->update(optionsButtonRect.toRect());
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

qreal FolderStatusDelegate::rootFolderHeightWithoutErrors() const
{
    if (!_ready) {
        return {};
    }
    const QFontMetricsF fm(_font);
    const QFontMetricsF aliasFm(_aliasFont);
    qreal h = _aliasMargin; // margin to top
    h += aliasFm.height(); // alias
    h += _margin; // between alias and local path
    h += fm.height(); // sync text

    // quota or progress bar
    h += _margin;
    h += fm.height(); // quota or progress bar
    h += _margin;
    h += fm.height(); // possible progress string
    return h;
}

void FolderStatusDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    option->state &= ~QStyle::State_HasFocus;
}

FolderStatusModel::ItemType FolderStatusDelegate::itemType(const QModelIndex &index)
{
    return index.siblingAtColumn(static_cast<int>(FolderStatusModel::Columns::ItemType)).data().value<FolderStatusModel::ItemType>();
}

void FolderStatusDelegate::drawStatusIcon(QPainter *painter, const QStyleOptionViewItem &option, const ItemData &data, const ItemLayout &layout) const
{
    const auto iconState = data.accountConnected ? QIcon::Normal : QIcon::Disabled;
    const auto iconVisualRect = QStyle::visualRect(option.direction, option.rect, layout.iconRect.toRect());

    data.spaceImage.paint(painter, iconVisualRect, Qt::AlignCenter, iconState);
    // Paint overlay in NormalState — on macOS disabled icons are semi-transparent,
    // drawing them on top of each other produces incorrect result.
    // Overlay is anchored to the bottom-right of iconRect; size and position are tunable constants.
    const qreal statusSize = layout.iconRect.width() * statusOverlaySizeRatioC;
    const qreal statusBottom = layout.iconRect.bottom() - layout.iconRect.height() * statusOverlayBottomInsetRatioC;
    const QRectF statusOverlayRect{QPointF(layout.iconRect.right() - statusSize, statusBottom - statusSize), QSizeF{statusSize, statusSize}};
    Theme::instance()
        ->themeIcon(QStringLiteral("states/%1").arg(data.statusIconName))
        .paint(painter, QStyle::visualRect(option.direction, option.rect, statusOverlayRect.toRect()), Qt::AlignCenter, QIcon::Normal);

    // Warning icon is only shown during sync — otherwise it is encoded in the status icon
    if (data.warningCount > 0 && data.syncOngoing) {
        const QRectF warningRect{layout.iconRect.bottomLeft() - QPointF(0, 17), QSizeF{16, 16}};
        Resources::getCoreIcon(QStringLiteral("warning"))
            .paint(painter, QStyle::visualRect(option.direction, option.rect, warningRect.toRect()), Qt::AlignCenter, iconState);
    }
}

void FolderStatusDelegate::drawTexts(QPainter *painter, const QStyleOptionViewItem &option, const ItemData &data, const ItemLayout &layout) const
{
    const QFontMetricsF subFm(_font);
    const QFontMetricsF aliasFm(_aliasFont);

    QPalette::ColorGroup cg = (option.state & QStyle::State_Enabled) ? QPalette::Normal : QPalette::Disabled;
    if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
        cg = QPalette::Inactive;

    const QPalette &pal = option.palette;
    const QColor textColor = (option.state & QStyle::State_Selected) ? pal.color(cg, QPalette::HighlightedText) : pal.color(cg, QPalette::Text);

    painter->setPen(textColor);
    painter->setFont(_aliasFont);
    painter->drawText(QStyle::visualRect(option.direction, option.rect, layout.aliasRect.toRect()), Qt::AlignLeft,
        aliasFm.elidedText(data.aliasText, Qt::ElideRight, layout.aliasRect.width()));

    painter->setFont(_font);
    painter->drawText(QStyle::visualRect(option.direction, option.rect, layout.localPathRect.toRect()), Qt::AlignLeft,
        subFm.elidedText(data.syncText, Qt::ElideRight, layout.localPathRect.width()));
}

void FolderStatusDelegate::drawProgressOrQuota(QPainter *painter, const QStyleOptionViewItem &option, const ItemData &data, const ItemLayout &layout) const
{
    const QFontMetricsF subFm(_font);
    const bool showProgress = !data.overallString.isEmpty() || !data.itemString.isEmpty();

    if (!showProgress) {
        if (data.totalQuota <= 0)
            return;

        painter->setFont(_progressFont);
        painter->drawText(QStyle::visualRect(option.direction, option.rect, layout.quotaTextRect.toRect()), Qt::AlignLeft | Qt::AlignVCenter,
            subFm.elidedText(tr("%1 of %2 in use").arg(Utility::octetsToString(data.usedQuota), Utility::octetsToString(data.totalQuota)), Qt::ElideRight,
                layout.quotaTextRect.width()));
        return;
    }

    painter->save();

    const auto progressRect = layout.quotaTextRect.marginsAdded({0, 0, 0, barHeightC + _margin + subFm.height()});
    const auto pBRect = QRectF{progressRect.topLeft(), QSizeF{progressRect.width() - 2 * _margin, barHeightC}};

    const QRectF barRect = QStyle::visualRect(option.direction, option.rect, pBRect.toRect());
    const qreal radius = barRect.height() / 2.0;
    const bool dark = Theme::instance()->isDarkTheme();

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::NoPen);
    painter->setBrush(dark ? QColor(0xFF, 0xFF, 0xFF, 51) : QColor(0x00, 0x00, 0x00, 38));
    painter->drawRoundedRect(barRect, radius, radius);

    if (data.overallPercent > 0) {
        QRectF fillRect = barRect;
        fillRect.setWidth(barRect.width() * data.overallPercent / 100.0);
        painter->setBrush(dark ? QColor(100, 181, 246) : QColor(25, 118, 210));
        painter->drawRoundedRect(fillRect, radius, radius);
    }

    const QRectF overallProgressRect{pBRect.bottomLeft() + QPointF{0, _margin}, QSizeF{progressRect.width(), subFm.height()}};
    painter->setFont(_progressFont);
    painter->drawText(QStyle::visualRect(option.direction, option.rect, overallProgressRect.toRect()), Qt::AlignLeft | Qt::AlignVCenter, data.overallString);

    painter->restore();
}

void FolderStatusDelegate::drawErrorBoxes(QPainter *painter, const QStyleOptionViewItem &option, const ItemData &data, const ItemLayout &layout) const
{
    if (data.conflictTexts.isEmpty() && data.errorTexts.isEmpty() && data.infoTexts.isEmpty() && data.warningTexts.isEmpty())
        return;

    const QFontMetricsF subFm(_font);
    const QFont errorFont = _font;
    qreal pos = option.rect.top() + rootFolderHeightWithoutErrors() + _margin;

    const auto drawTextBox = [&](const QStringList &texts, QColor color) {
        QRectF rect = layout.quotaTextRect;
        rect.setLeft(layout.iconRect.left());
        rect.setTop(pos);
        rect.setHeight(texts.count() * subFm.height() + 2 * _margin);
        rect.setRight(option.rect.right() - _margin);

        painter->save();
        painter->setBrush(color);
        painter->setPen(QColor(0xaa, 0xaa, 0xaa));
        painter->drawRoundedRect(QStyle::visualRect(option.direction, option.rect, rect.toRect()), 4, 4);
        painter->setPen(Qt::white);
        painter->setFont(errorFont);

        QRect textRect(rect.left() + _margin, rect.top() + _margin, rect.width() - 2 * _margin, subFm.height());
        for (const auto &text : texts) {
            painter->drawText(
                QStyle::visualRect(option.direction, option.rect, textRect), Qt::AlignLeft, subFm.elidedText(text, Qt::ElideLeft, textRect.width()));
            textRect.translate(0, textRect.height());
        }
        painter->restore();
        pos = rect.bottom() + _margin;
    };

    if (!data.conflictTexts.isEmpty())
        drawTextBox(data.conflictTexts, QColor(0xba, 0xba, 0x4d));
    if (!data.warningTexts.isEmpty())
        drawTextBox(data.warningTexts, QColor(0xe0, 0x9b, 0x2d));
    if (!data.errorTexts.isEmpty())
        drawTextBox(data.errorTexts, QColor(0xbb, 0x4d, 0x4d));
    if (!data.infoTexts.isEmpty())
        drawTextBox(data.infoTexts, QColor(0x4d, 0x4d, 0xba));
}

void FolderStatusDelegate::drawOptionsButton(QPainter *painter, const QStyleOptionViewItem &option, bool hovered, bool pressed) const
{
    const auto optionsButtonRect = computeOptionsButtonRect(option.rect);

    QStyleOptionToolButton btnOpt;
    btnOpt.state = option.state;
    btnOpt.state &= ~(QStyle::State_Selected | QStyle::State_HasFocus);
    btnOpt.state |= QStyle::State_Raised;
    btnOpt.state.setFlag(QStyle::State_MouseOver, hovered);
    btnOpt.state.setFlag(QStyle::State_Sunken, pressed);
    btnOpt.arrowType = Qt::NoArrow;
    btnOpt.subControls = QStyle::SC_ToolButton;
    btnOpt.rect = QStyle::visualRect(option.direction, option.rect, optionsButtonRect.toRect());
    btnOpt.icon = Resources::getCoreIcon(QStringLiteral("more"));
    const int iconSize = QApplication::style()->pixelMetric(QStyle::PM_ButtonIconSize);
    btnOpt.iconSize = QSize(iconSize, iconSize);

    ToolButtonDots::drawButton(&btnOpt, painter, Theme::instance()->isDarkTheme());
}

FolderStatusDelegate::ItemLayout FolderStatusDelegate::computeLayout(const QStyleOptionViewItem &option, bool centerTexts) const
{
    const QFontMetricsF subFm(_font);
    const QFontMetricsF aliasFm(_aliasFont);

    ItemLayout l;
    l.statusRect = QRectF{option.rect}.adjusted(0, 0, 0, rootFolderHeightWithoutErrors() - option.rect.height());
    l.iconRect = QRectF{l.statusRect.topLeft(), QSizeF{l.statusRect.height(), l.statusRect.height()}}.marginsRemoved(
        {_aliasMargin, _aliasMargin, _aliasMargin, _aliasMargin});

    const auto infoRect =
        QRectF{l.iconRect.topRight(), QSizeF{l.statusRect.width() - l.iconRect.width(), l.iconRect.height()}}.marginsRemoved({_aliasMargin, 0, 0, 0});
    l.aliasRect = QRectF{infoRect.topLeft(), QSizeF{infoRect.width(), aliasFm.height()}};
    l.optionsButtonRect = computeOptionsButtonRect(option.rect);

    if (centerTexts) {
        const qreal textBlockHeight = aliasFm.height() + _margin + subFm.height();
        const qreal offset = (infoRect.height() - textBlockHeight) / 2.0;
        l.aliasRect.translate(0, offset);
    }

    const auto marginOffset = QPointF{0, _margin};
    l.localPathRect = QRectF{l.aliasRect.bottomLeft() + marginOffset, QSizeF{l.aliasRect.width(), subFm.height()}};
    l.quotaTextRect = [&] {
        QRectF r{l.localPathRect.bottomLeft() + marginOffset, QSizeF{l.aliasRect.width(), subFm.height()}};
        r.setRight(l.optionsButtonRect.left() - _margin);
        return r;
    }();

    return l;
}

FolderStatusDelegate::ItemData FolderStatusDelegate::fetchData(const QModelIndex &index)
{
    const auto col = [&](FolderStatusModel::Columns c) { return index.siblingAtColumn(static_cast<int>(c)).data(); };

    ItemData d;
    d.statusIconName = col(FolderStatusModel::Columns::FolderStatusIconRole).toString();
    d.aliasText = col(FolderStatusModel::Columns::HeaderRole).toString();
    d.syncText = col(FolderStatusModel::Columns::FolderSyncText).toString();
    d.conflictTexts = col(FolderStatusModel::Columns::FolderConflictMsg).toStringList();
    d.errorTexts = col(FolderStatusModel::Columns::FolderErrorMsg).toStringList();
    d.infoTexts = col(FolderStatusModel::Columns::FolderInfoMsg).toStringList();
    d.warningTexts = col(FolderStatusModel::Columns::FolderWarningMsg).toStringList();
    d.spaceImage = col(FolderStatusModel::Columns::FolderImage).value<QIcon>();
    d.overallPercent = col(FolderStatusModel::Columns::SyncProgressOverallPercent).toInt();
    d.overallString = col(FolderStatusModel::Columns::SyncProgressOverallString).toString();
    d.itemString = col(FolderStatusModel::Columns::SyncProgressItemString).toString();
    d.warningCount = col(FolderStatusModel::Columns::WarningCount).toInt();
    d.syncOngoing = col(FolderStatusModel::Columns::SyncRunning).toBool();
    d.accountConnected = col(FolderStatusModel::Columns::FolderAccountConnected).toBool();
    d.totalQuota = col(FolderStatusModel::Columns::QuotaTotal).value<int64_t>();
    d.usedQuota = col(FolderStatusModel::Columns::QuotaUsed).value<int64_t>();
    return d;
}

void FolderStatusDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() != 0)
        return;

    if (itemType(index) != FolderStatusModel::RootFolder)
        return QStyledItemDelegate::paint(painter, option, index);

    updateFont(option.font);

    const auto data = fetchData(index);
    const bool showProgress = !data.overallString.isEmpty() || !data.itemString.isEmpty();
    const auto layout = computeLayout(option, !showProgress && data.totalQuota <= 0);

    painter->save();
    drawStatusIcon(painter, option, data, layout);
    drawTexts(painter, option, data, layout);
    drawProgressOrQuota(painter, option, data, layout);
    drawErrorBoxes(painter, option, data, layout);
    painter->restore();

    drawOptionsButton(painter, option, hoveredRow_ == index.row(), pressedRow_ == index.row());
}

QRectF FolderStatusDelegate::computeOptionsButtonRect(QRectF within) const
{
    if (!_ready) {
        return {};
    }
    within.setHeight(FolderStatusDelegate::rootFolderHeightWithoutErrors());

    return {{within.right() - toolbutton_size.width() - QApplication::style()->pixelMetric(QStyle::PM_LayoutRightMargin),
                within.top() + within.height() / 2 - toolbutton_size.height() / 2},
        toolbutton_size};
}

QRect FolderStatusDelegate::getCheckboxRect() const
{
    return checkboxRect;
}

QRectF FolderStatusDelegate::errorsListRect(QRectF within, const QModelIndex &index) const
{
    if (!_ready) {
        return {};
    }
    const QFontMetrics fm(_font);
    within.setTop(within.top() + FolderStatusDelegate::rootFolderHeightWithoutErrors() + _margin);
    qreal h = 0;
    for (auto column :
        {FolderStatusModel::Columns::FolderConflictMsg, FolderStatusModel::Columns::FolderErrorMsg, FolderStatusModel::Columns::FolderWarningMsg}) {
        const auto msgs = index.siblingAtColumn(static_cast<int>(column)).data().toStringList();
        if (!msgs.isEmpty()) {
            h += _margin + 2 * _margin + msgs.count() * fm.height() + _margin;
        }
    }
    within.setHeight(h);
    return within;
}

void FolderStatusDelegate::updateFont(const QFont &font) const
{
    if (!_ready || _font != font) {
        _ready = true;
        _font = font;

        _aliasFont = font;
        _aliasFont.setBold(true);
        _aliasFont.setPointSizeF(font.pointSizeF() + 2);

        _progressFont = font;
        _progressFont.setPointSize(font.pointSize() - 2);

        _margin = QFontMetricsF(_font).height() / 4.0;
        _aliasMargin = QFontMetricsF(_aliasFont).height() / 2.0;
    }
}


} // namespace APP
