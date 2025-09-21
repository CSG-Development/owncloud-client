#include "checkboxres.h"


namespace {

class ControlIcons
{
public:
    QIcon normal;
    QIcon hover;
    QIcon pressed;
    QIcon disabled;
};

ControlIcons checkboxUncheckedIcons;
ControlIcons checkboxCheckedIcons;
ControlIcons checkboxPartialIcons;

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

namespace CheckboxRes {

void init()
{
    static bool is_initialized = false;
    if (!is_initialized) {
        checkboxUncheckedIcons = {
            QIcon(QStringLiteral(":/res/checkbox/normal/unchecked.svg")),
            QIcon(QStringLiteral(":/res/checkbox/hover/unchecked.svg")),
            QIcon(QStringLiteral(":/res/checkbox/pressed/unchecked.svg")),
            QIcon(QStringLiteral(":/res/checkbox/disabled/unchecked.svg")),
        };

        checkboxCheckedIcons = {
            QIcon(QStringLiteral(":/res/checkbox/normal/checked.svg")),
            QIcon(QStringLiteral(":/res/checkbox/hover/checked.svg")),
            QIcon(QStringLiteral(":/res/checkbox/pressed/checked.svg")),
            QIcon(QStringLiteral(":/res/checkbox/disabled/checked.svg")),
        };

        checkboxPartialIcons = {
            QIcon(QStringLiteral(":/res/checkbox/normal/partial.svg")),
            QIcon(QStringLiteral(":/res/checkbox/hover/partial.svg")),
            QIcon(QStringLiteral(":/res/checkbox/pressed/partial.svg")),
            QIcon(QStringLiteral(":/res/checkbox/disabled/partial.svg")),
        };
        is_initialized = true;
    }
}

const QIcon& getChkIcon(QStyle::State state)
{
    if (state.testFlag(QStyle::State_On)) {
        return styleIcon(checkboxCheckedIcons, state);
    }
    return styleIcon(checkboxUncheckedIcons, state);
}

} // namespace CheckboxRes
