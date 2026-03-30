/*
 * Copyright (C) by Klaas Freitag <freitag@kde.org>
 * Copyright (C) by Olivier Goffart <ogoffart@woboq.com>
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

#pragma once
#include "folderstatusmodel.h"
#include <QStyledItemDelegate>

namespace APP {

class FolderStatusModel;

/**
 * @brief The FolderStatusDelegate class
 * @ingroup gui
 */
class FolderStatusDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    FolderStatusDelegate(QObject *parent);

    void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const override;
    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const override;

    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

    /**
     * return the position of the option button within the item
     */
    QRectF computeOptionsButtonRect(QRectF within) const;
    QRect getCheckboxRect() const;
    QRectF errorsListRect(QRectF within, const QModelIndex &) const;
    qreal rootFolderHeightWithoutErrors() const;

    void setModel(FolderStatusModel* model) {model_ = model;}
    void resetButtonState() { hoveredRow_ = -1; pressedRow_ = -1; }

protected:
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;

private:
    struct ItemData {
        QString statusIconName;
        QString aliasText;
        QString syncText;
        QStringList conflictTexts;
        QStringList errorTexts;
        QStringList infoTexts;
        QIcon spaceImage;
        int overallPercent = 0;
        QString overallString;
        QString itemString;
        int warningCount = 0;
        bool syncOngoing = false;
        bool accountConnected = false;
        int64_t totalQuota = 0;
        int64_t usedQuota = 0;
    };

    struct ItemLayout {
        QRectF statusRect;
        QRectF iconRect;
        QRectF aliasRect;
        QRectF localPathRect;
        QRectF quotaTextRect;
        QRectF optionsButtonRect;
    };

    static FolderStatusModel::ItemType itemType(const QModelIndex &index);
    static ItemData fetchData(const QModelIndex &index);
    ItemLayout computeLayout(const QStyleOptionViewItem &option, bool centerTexts = false) const;

    void drawStatusIcon(QPainter *painter, const QStyleOptionViewItem &option, const ItemData &data, const ItemLayout &layout) const;
    void drawTexts(QPainter *painter, const QStyleOptionViewItem &option, const ItemData &data, const ItemLayout &layout) const;
    void drawProgressOrQuota(QPainter *painter, const QStyleOptionViewItem &option, const ItemData &data, const ItemLayout &layout) const;
    void drawErrorBoxes(QPainter *painter, const QStyleOptionViewItem &option, const ItemData &data, const ItemLayout &layout) const;
    void drawOptionsButton(QPainter *painter, const QStyleOptionViewItem &option, bool hovered, bool pressed) const;

    static QString addFolderText(bool useSapces);

    // a workaround for a design flaw of the class
    // we need to know the actual font for most computations
    // the font is only set in paint and sizeHint
    void updateFont(const QFont &font) const;

    mutable QFont _aliasFont;
    mutable QFont _progressFont;
    mutable QFont _font;
    mutable qreal _margin = 0;
    mutable qreal _aliasMargin = 0;
    mutable bool _ready = false;
    int hoveredRow_ = -1;
    int pressedRow_ = -1;
    FolderStatusModel* model_ = nullptr;
    QRect checkboxRect;
};

} // namespace APP
