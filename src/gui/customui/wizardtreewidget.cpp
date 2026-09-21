#include "wizardtreewidget.h"

#include "libsync/apppalette.h"
#include "libsync/theme.h"

#include <QApplication>
#include <QCursor>
#include <QHeaderView>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScrollBar>
#include <QStyledItemDelegate>

namespace {

constexpr int kIndentation = 24;
constexpr int kRowHeight = 34;
constexpr int kHighlightHeight = 26;
constexpr int kHighlightLeftPad = 6;
constexpr int kHighlightRightPad = 12;
constexpr int kHighlightRadius = 4;
constexpr int kHoverRadius = 3;
constexpr int kAccentWidth = 3;
constexpr int kAccentHeight = 16;
constexpr int kIconCenterOffset = 16;
constexpr int kTextOffset = 36;
constexpr int kCheckOffset = 8;
constexpr int kCheckSize = 16;
constexpr int kCheckExtra = 24;
constexpr int kRootIconSize = 28;
constexpr int kChildIconSize = 16;
constexpr int kCellPadding = 8;
constexpr qreal kDisabledOpacity = 0.4;
constexpr int kCheckHitMargin = 4;
constexpr int kNameColumn = 0;
constexpr qreal kChevronHalfLong = 3.5;
constexpr qreal kChevronHalfShort = 1.75;
constexpr qreal kChevronPenWidth = 1.0;
const QPoint kOutsidePos(-1, -1);

struct RowLayout
{
    QRect check;
    QRect icon;
    QRect text;
    int width = 0;
};

RowLayout rowLayout(const QRect &itemRect, const QFontMetrics &metrics, const QString &text, bool isRoot, bool checkable)
{
    RowLayout layout;
    const int centerY = itemRect.top() + itemRect.height() / 2;
    const int extra = checkable ? kCheckExtra : 0;
    if (checkable) {
        layout.check = QRect(itemRect.left() + kCheckOffset, centerY - kCheckSize / 2, kCheckSize, kCheckSize);
    }
    const int iconSize = isRoot ? kRootIconSize : kChildIconSize;
    const int iconCenterX = itemRect.left() + kIconCenterOffset + extra;
    layout.icon = QRect(iconCenterX - iconSize / 2, centerY - iconSize / 2, iconSize, iconSize);
    const int textWidth = metrics.horizontalAdvance(text);
    layout.text = QRect(itemRect.left() + kTextOffset + extra, itemRect.top(), textWidth, itemRect.height());
    layout.width = kTextOffset + extra + textWidth + kHighlightRightPad;
    return layout;
}

QString checkIconPath(Qt::CheckState state, bool enabled, bool isDark)
{
#ifdef Q_OS_MACOS
    const auto platform = QStringLiteral("mac");
#else
    const auto platform = QStringLiteral("win");
#endif
    const auto name = state == Qt::Checked ? QStringLiteral("checked")
                                           : (state == Qt::PartiallyChecked ? QStringLiteral("partial") : QStringLiteral("unchecked"));
    return QStringLiteral(":/res/checkbox_%1/%2/%3/%4.svg")
        .arg(platform, isDark ? QStringLiteral("dark") : QStringLiteral("light"), enabled ? QStringLiteral("normal") : QStringLiteral("disabled"), name);
}

class WizardTreeDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        if (index.column() != kNameColumn) {
            return {opt.fontMetrics.horizontalAdvance(opt.text) + 2 * kCellPadding, kRowHeight};
        }
        const auto layout = rowLayout(opt.rect, opt.fontMetrics, opt.text, !index.parent().isValid(), isCheckable(index));
        return {layout.width, kRowHeight};
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        const bool isDark = APP::Theme::instance()->isDarkTheme();
        const bool enabled = opt.state & QStyle::State_Enabled;

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setFont(opt.font);
        painter->setPen(APP::AppPalette::color(APP::ColorToken::TreeItemText, isDark));
        if (!enabled) {
            painter->setOpacity(kDisabledOpacity);
        }

        if (index.column() != kNameColumn) {
            const QRect textRect = opt.rect.adjusted(kCellPadding, 0, -kCellPadding, 0);
            painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, opt.fontMetrics.elidedText(opt.text, Qt::ElideRight, textRect.width()));
            painter->restore();
            return;
        }

        const auto layout = rowLayout(opt.rect, opt.fontMetrics, opt.text, !index.parent().isValid(), isCheckable(index));
        if (!layout.check.isNull()) {
            const auto state = static_cast<Qt::CheckState>(index.data(Qt::CheckStateRole).toInt());
            QIcon(checkIconPath(state, enabled, isDark)).paint(painter, layout.check);
        }
        opt.icon.paint(painter, layout.icon, Qt::AlignCenter, enabled ? QIcon::Normal : QIcon::Disabled);

        const QRect textRect(layout.text.left(), layout.text.top(), qMax(0, qMin(layout.text.width(), opt.rect.right() - layout.text.left())), layout.text.height());
        painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, opt.fontMetrics.elidedText(opt.text, Qt::ElideRight, textRect.width()));
        painter->restore();
    }

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override
    {
        const auto type = event->type();
        if (type != QEvent::MouseButtonRelease && type != QEvent::MouseButtonDblClick) {
            return QStyledItemDelegate::editorEvent(event, model, option, index);
        }
        if (index.column() != kNameColumn || !isCheckable(index) || !(index.flags() & Qt::ItemIsUserCheckable) || !(option.state & QStyle::State_Enabled)) {
            return false;
        }
        const auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() != Qt::LeftButton) {
            return false;
        }

        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        const auto layout = rowLayout(opt.rect, opt.fontMetrics, opt.text, !index.parent().isValid(), true);
        if (!layout.check.adjusted(-kCheckHitMargin, -kCheckHitMargin, kCheckHitMargin, kCheckHitMargin).contains(mouseEvent->position().toPoint())) {
            return false;
        }
        if (type == QEvent::MouseButtonDblClick) {
            return true;
        }

        const auto state = static_cast<Qt::CheckState>(index.data(Qt::CheckStateRole).toInt());
        const auto next = (state == Qt::Checked) ? Qt::Unchecked : Qt::Checked;
        return model->setData(index, next, Qt::CheckStateRole);
    }

private:
    static bool isCheckable(const QModelIndex &index)
    {
        return index.data(Qt::CheckStateRole).isValid();
    }
};

void drawChevron(QPainter *painter, const QPointF &center, bool pointDown, const QColor &color)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(color, kChevronPenWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->setBrush(Qt::NoBrush);

    QPainterPath path;
    if (pointDown) {
        path.moveTo(center.x() - kChevronHalfLong, center.y() - kChevronHalfShort);
        path.lineTo(center.x(), center.y() + kChevronHalfShort);
        path.lineTo(center.x() + kChevronHalfLong, center.y() - kChevronHalfShort);
    } else {
        path.moveTo(center.x() - kChevronHalfShort, center.y() - kChevronHalfLong);
        path.lineTo(center.x() + kChevronHalfShort, center.y());
        path.lineTo(center.x() - kChevronHalfShort, center.y() + kChevronHalfLong);
    }
    painter->drawPath(path);
    painter->restore();
}

} // namespace

WizardTreeWidget::WizardTreeWidget(QWidget *parent)
    : QTreeWidget(parent)
{
    static const auto resourcesLoaded = [] {
        Q_INIT_RESOURCE(customui_res);
        return true;
    }();
    Q_UNUSED(resourcesLoaded)

    setItemDelegate(new WizardTreeDelegate(this));
    setIndentation(kIndentation);
    setRootIsDecorated(true);
    setAnimated(false);
    setUniformRowHeights(true);
    setAlternatingRowColors(false);
    setFrameShape(QFrame::NoFrame);
    setMouseTracking(true);
    viewport()->setMouseTracking(true);

    QPalette pal = palette();
    pal.setColor(QPalette::Highlight, Qt::transparent);
    pal.setColor(QPalette::Inactive, QPalette::Highlight, Qt::transparent);
    setPalette(pal);

    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, [this] {
        updateHover(viewport()->mapFromGlobal(QCursor::pos()));
    });
    connect(APP::Theme::instance(), &APP::Theme::themeChanged, viewport(), qOverload<>(&QWidget::update));
}

QIcon WizardTreeWidget::rootIcon()
{
    static const auto resourcesLoaded = [] {
        Q_INIT_RESOURCE(customui_res);
        return true;
    }();
    Q_UNUSED(resourcesLoaded)

    return QIcon(QStringLiteral(":/res/tree_logo.svg"));
}

QRect WizardTreeWidget::highlightRect(const QModelIndex &index) const
{
    const QModelIndex first = index.siblingAtColumn(kNameColumn);
    const QRect item = visualRect(first);
    if (!first.isValid() || !item.isValid()) {
        return {};
    }

    QStyleOptionViewItem opt;
    initViewItemOption(&opt);
    opt.rect = item;
    const int width = itemDelegate()->sizeHint(opt, first).width();

    const int left = qMax(0, item.left() - kIndentation - kHighlightLeftPad);
    const int top = item.top() + (item.height() - kHighlightHeight) / 2;
    return QRect(left, top, item.left() + width - left, kHighlightHeight);
}

void WizardTreeWidget::updateHover(const QPoint &pos)
{
    QModelIndex hovered = indexAt(pos).siblingAtColumn(kNameColumn);
    if (hovered.isValid() && !highlightRect(hovered).contains(pos)) {
        hovered = QModelIndex();
    }
    if (hovered == _hoverIndex) {
        return;
    }
    _hoverIndex = hovered;
    viewport()->update();
}

void WizardTreeWidget::mouseMoveEvent(QMouseEvent *event)
{
    QTreeWidget::mouseMoveEvent(event);
    updateHover(event->position().toPoint());
}

bool WizardTreeWidget::viewportEvent(QEvent *event)
{
    if (event->type() == QEvent::Leave) {
        updateHover(kOutsidePos);
    }
    return QTreeWidget::viewportEvent(event);
}

void WizardTreeWidget::drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const QModelIndex first = index.siblingAtColumn(kNameColumn);
    const QRect highlight = highlightRect(first);
    if (highlight.isValid()) {
        const bool isDark = APP::Theme::instance()->isDarkTheme();
        const bool selected = selectionModel() && selectionModel()->isSelected(first);
        const bool hovered = _hoverIndex.isValid() && first == _hoverIndex;

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(Qt::NoPen);

        if (hovered || selected) {
            const auto token = hovered ? APP::ColorToken::TreeItemHover : APP::ColorToken::TreeItemSelected;
            const int radius = hovered && !selected ? kHoverRadius : kHighlightRadius;
            painter->setBrush(APP::AppPalette::color(token, isDark));
            painter->drawRoundedRect(highlight, radius, radius);
        }
        if (selected) {
            painter->setBrush(APP::AppPalette::color(APP::ColorToken::Primary, isDark));
            const QRectF accent(highlight.left(), highlight.top() + (highlight.height() - kAccentHeight) / 2.0, kAccentWidth, kAccentHeight);
            painter->drawRoundedRect(accent, kAccentWidth / 2.0, kAccentWidth / 2.0);
        }
        painter->restore();
    }

    QStyleOptionViewItem opt = option;
    opt.state &= ~(QStyle::State_Selected | QStyle::State_MouseOver | QStyle::State_HasFocus);
    opt.showDecorationSelected = false;

    const QRect itemRect = visualRect(first);
    const auto *delegate = itemDelegate();
    for (int visual = 0; visual < header()->count(); ++visual) {
        const int column = header()->logicalIndex(visual);
        if (header()->isSectionHidden(column)) {
            continue;
        }
        QRect cell(columnViewportPosition(column), option.rect.top(), columnWidth(column), option.rect.height());
        if (column == kNameColumn) {
            drawBranches(painter, QRect(cell.left(), cell.top(), itemRect.left() - cell.left(), cell.height()), first);
            cell.setLeft(itemRect.left());
        }
        opt.rect = cell;
        delegate->paint(painter, opt, index.siblingAtColumn(column));
    }
}

void WizardTreeWidget::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    if (!model() || !model()->hasChildren(index)) {
        return;
    }

    const bool isDark = APP::Theme::instance()->isDarkTheme();
    const QPointF center(rect.right() + 1 - kIndentation / 2.0, rect.top() + rect.height() / 2.0);
    drawChevron(painter, center, isExpanded(index), APP::AppPalette::color(APP::ColorToken::TreeItemText, isDark));
}
