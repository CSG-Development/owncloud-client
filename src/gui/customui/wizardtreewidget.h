#pragma once

#include <QIcon>
#include <QPersistentModelIndex>
#include <QTreeWidget>

class WizardTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit WizardTreeWidget(QWidget *parent = nullptr);

    static QIcon rootIcon();

protected:
    void drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const override;
    void mouseMoveEvent(QMouseEvent *event) override;
    bool viewportEvent(QEvent *event) override;

private:
    QRect highlightRect(const QModelIndex &index) const;
    void updateHover(const QPoint &pos);

    QPersistentModelIndex _hoverIndex;
};
