#pragma once
#include <QTreeView>

class FolderTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit FolderTreeView(QWidget *parent = nullptr);

    void setSelectionColor(const QColor &color);

protected:
    void drawBranches(QPainter *painter, const QRect &rect,
                      const QModelIndex &index) const override;

private:
    QColor _selectionColor{25, 118, 210, 61};
};
