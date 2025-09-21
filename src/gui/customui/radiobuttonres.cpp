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

ControlIcons radiobuttonUncheckedIcons;
ControlIcons radiobuttonCheckedIcons;

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
        radiobuttonUncheckedIcons = {
            QIcon(QStringLiteral(":/res/radiobutton/normal/unchecked.svg")),
            QIcon(QStringLiteral(":/res/radiobutton/hover/unchecked.svg")),
            QIcon(QStringLiteral(":/res/radiobutton/pressed/unchecked.svg")),
            QIcon(QStringLiteral(":/res/radiobutton/disabled/unchecked.svg")),
        };

        radiobuttonCheckedIcons = {
            QIcon(QStringLiteral(":/res/radiobutton/normal/checked.svg")),
            QIcon(QStringLiteral(":/res/radiobutton/hover/checked.svg")),
            QIcon(QStringLiteral(":/res/radiobutton/pressed/checked.svg")),
            QIcon(QStringLiteral(":/res/radiobutton/disabled/checked.svg")),
        };

        is_initialized = true;
    }
}

const QIcon& getRbIcon(QStyle::State state)
{
    if (state.testFlag(QStyle::State_On)) {
        return styleIcon(radiobuttonCheckedIcons, state);
    }
    return styleIcon(radiobuttonUncheckedIcons, state);
}

} // namespace RadiobuttonRes
