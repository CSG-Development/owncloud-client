#include "radiobuttonres.h"


namespace {

class ControlIcons
{
public:
    QIcon normal;
    QIcon hover;
    QIcon pressed;
    QIcon disabled;
};

std::array<ControlIcons,2> radiobuttonUncheckedIcons;
std::array<ControlIcons,2> radiobuttonCheckedIcons;

const QIcon& styleIcon(const ControlIcons &icons, QStyle::State state)
{
    if (!state.testFlag(QStyle::State_Enabled)) {
        return icons.disabled;
    }
    if (!state.testFlag(QStyle::State_MouseOver)) {
        return icons.hover;
    }
    if (!state.testFlag(QStyle::State_Sunken)) {
        return icons.pressed;
    }

    return icons.normal;
}

}

namespace RadiobuttonRes {

void init()
{
    static bool is_initialized = false;
    if (!is_initialized) {
        radiobuttonUncheckedIcons[0] = {
            QIcon(QStringLiteral(":/res/radiobutton/light/normal/unchecked.svg")),
            QIcon(QStringLiteral(":/res/radiobutton/light/hover/unchecked.svg")),
            QIcon(QStringLiteral(":/res/radiobutton/light/pressed/unchecked.svg")),
            QIcon(QStringLiteral(":/res/radiobutton/light/disabled/unchecked.svg")),
        };

        radiobuttonUncheckedIcons[1] = {
            QIcon(QStringLiteral(":/res/radiobutton/dark/normal/unchecked.svg")),
            QIcon(QStringLiteral(":/res/radiobutton/dark/hover/unchecked.svg")),
            QIcon(QStringLiteral(":/res/radiobutton/dark/pressed/unchecked.svg")),
            QIcon(QStringLiteral(":/res/radiobutton/dark/disabled/unchecked.svg")),
        };

        radiobuttonCheckedIcons[0] = {
            QIcon(QStringLiteral(":/res/radiobutton/light/normal/checked.svg")),
            QIcon(QStringLiteral(":/res/radiobutton/light/hover/checked.svg")),
            QIcon(QStringLiteral(":/res/radiobutton/light/pressed/checked.svg")),
            QIcon(QStringLiteral(":/res/radiobutton/light/disabled/checked.svg")),
        };

        radiobuttonCheckedIcons[1] = {
            QIcon(QStringLiteral(":/res/radiobutton/dark/normal/checked.svg")),
            QIcon(QStringLiteral(":/res/radiobutton/dark/hover/checked.svg")),
            QIcon(QStringLiteral(":/res/radiobutton/dark/pressed/checked.svg")),
            QIcon(QStringLiteral(":/res/radiobutton/dark/disabled/checked.svg")),
        };

        is_initialized = true;
    }
}

const QIcon& getRbIcon(QStyle::State state, bool isDark)
{
    if (state.testFlag(QStyle::State_On)) {
        return styleIcon(radiobuttonCheckedIcons[isDark ? 1 : 0], state);
    }
    return styleIcon(radiobuttonUncheckedIcons[isDark ? 1 : 0], state);
}

} // namespace RadiobuttonRes
