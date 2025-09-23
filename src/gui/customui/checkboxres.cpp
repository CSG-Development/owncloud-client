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

std::array<ControlIcons,2> checkboxUncheckedIcons;  // 0 light, 1 dark
std::array<ControlIcons,2> checkboxCheckedIcons;
std::array<ControlIcons,2> checkboxPartialIcons;

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
        checkboxUncheckedIcons[0] = {
            QIcon(QStringLiteral(":/res/checkbox/light/normal/unchecked.svg")),
            QIcon(QStringLiteral(":/res/checkbox/light/hover/unchecked.svg")),
            QIcon(QStringLiteral(":/res/checkbox/light/pressed/unchecked.svg")),
            QIcon(QStringLiteral(":/res/checkbox/light/disabled/unchecked.svg")),
        };

        checkboxUncheckedIcons[1] = {
            QIcon(QStringLiteral(":/res/checkbox/dark/normal/unchecked.svg")),
            QIcon(QStringLiteral(":/res/checkbox/dark/hover/unchecked.svg")),
            QIcon(QStringLiteral(":/res/checkbox/dark/pressed/unchecked.svg")),
            QIcon(QStringLiteral(":/res/checkbox/dark/disabled/unchecked.svg")),
        };

        checkboxCheckedIcons[0] = {
            QIcon(QStringLiteral(":/res/checkbox/light/normal/checked.svg")),
            QIcon(QStringLiteral(":/res/checkbox/light/hover/checked.svg")),
            QIcon(QStringLiteral(":/res/checkbox/light/pressed/checked.svg")),
            QIcon(QStringLiteral(":/res/checkbox/light/disabled/checked.svg")),
        };

        checkboxCheckedIcons[1] = {
            QIcon(QStringLiteral(":/res/checkbox/dark/normal/checked.svg")),
            QIcon(QStringLiteral(":/res/checkbox/dark/hover/checked.svg")),
            QIcon(QStringLiteral(":/res/checkbox/dark/pressed/checked.svg")),
            QIcon(QStringLiteral(":/res/checkbox/dark/disabled/checked.svg")),
        };

        checkboxPartialIcons[0] = {
            QIcon(QStringLiteral(":/res/checkbox/light/normal/partial.svg")),
            QIcon(QStringLiteral(":/res/checkbox/light/hover/partial.svg")),
            QIcon(QStringLiteral(":/res/checkbox/light/pressed/partial.svg")),
            QIcon(QStringLiteral(":/res/checkbox/light/disabled/partial.svg")),
        };

        checkboxPartialIcons[1] = {
            QIcon(QStringLiteral(":/res/checkbox/dark/normal/partial.svg")),
            QIcon(QStringLiteral(":/res/checkbox/dark/hover/partial.svg")),
            QIcon(QStringLiteral(":/res/checkbox/dark/pressed/partial.svg")),
            QIcon(QStringLiteral(":/res/checkbox/dark/disabled/partial.svg")),
        };

        is_initialized = true;
    }
}

const QIcon& getChkIcon(QStyle::State state, bool isDark)
{
    if (state.testFlag(QStyle::State_On)) {
        return styleIcon(checkboxCheckedIcons[isDark ? 1 : 0], state);
    }
    return styleIcon(checkboxUncheckedIcons[isDark ? 1 : 0], state);
}

} // namespace CheckboxRes
