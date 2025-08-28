#pragma once

#include <QStyleOptionToolButton>

namespace CUR {

class ToolButtonDots
{
public:
    static void drawButton(QStyleOptionToolButton* opt, QPainter* painter, bool isDark = false);
};

} // namespace CUR
