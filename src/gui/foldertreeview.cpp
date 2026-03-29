#include "foldertreeview.h"
#include "folderstatusmodel.h"
#include <QPainter>
#include <QPainterPath>

namespace {

constexpr int root_left_margin = 12;
constexpr int chevron_width  = 9;
constexpr int chevron_height = 5;

// Draws a right-pointing (>) or down-pointing (v) chevron inside area.
void drawChevron(QPainter *painter, const QRect &area, bool pointDown, const QColor &color)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(color, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->setBrush(Qt::NoBrush);

    const QRectF r(area);
    QPainterPath path;
    if (pointDown) {
        path.moveTo(r.left(),           r.top());
        path.lineTo(r.center().x(),     r.bottom());
        path.lineTo(r.right(),          r.top());
    } else {
        path.moveTo(r.left(),  r.top());
        path.lineTo(r.right(), r.center().y());
        path.lineTo(r.left(),  r.bottom());
    }
    painter->drawPath(path);
    painter->restore();
}

} // namespace

FolderTreeView::FolderTreeView(QWidget *parent)
    : QTreeView(parent)
{
}

void FolderTreeView::setSelectionColor(const QColor &color)
{
    _selectionColor = color;
}

void FolderTreeView::drawBranches(QPainter *painter, const QRect &rect,
                                   const QModelIndex &index) const
{
    const auto itemType = index
                              .siblingAtColumn(static_cast<int>(APP::FolderStatusModel::Columns::ItemType))
                              .data()
                              .value<APP::FolderStatusModel::ItemType>();

    if (itemType != APP::FolderStatusModel::RootFolder) {
        const bool isSelected = selectionModel()
        && (selectionModel()->isSelected(index)
           || selectionModel()->currentIndex() == index);

        // Always clear to base color first to remove platform highlight
        painter->fillRect(rect, viewport()->palette().color(QPalette::Base));

        // Then overlay selection color on top
        if (isSelected) {
            painter->fillRect(rect, _selectionColor);
        }
    }

    // Draw expand/collapse chevron centered in the rightmost indent column
    if (model() && model()->hasChildren(index)) {
        const bool open = isExpanded(index);
        // > is taller than wide, v is wider than tall
        const int arrowW = open ? chevron_width : chevron_height;
        const int arrowH = open ? chevron_height  : chevron_width;
        const int colLeft = rect.right() + 1 - indentation();
        // Root items start at x=0 — add left padding to match design
        const int leftPad = (itemType == APP::FolderStatusModel::RootFolder) ? root_left_margin : 0;
        const int x = colLeft + leftPad + (indentation() - leftPad - arrowW) / 2;
        const int y = rect.top() + (rect.height() - arrowH) / 2;
        drawChevron(painter, QRect(x, y, arrowW, arrowH), open,
                    viewport()->palette().color(QPalette::Text));
    }
}
