#pragma once

#include <QStyleOptionToolButton>

namespace APP {

class ToolButtonDots
{
public:
    static void drawButton(QStyleOptionToolButton* opt, QPainter* painter, bool isDark = false);
};

} // namespace APP
