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
#ifdef Q_OS_MACOS
        checkboxUncheckedIcons[0] = {
            QIcon(QStringLiteral(":/res/checkbox_mac/light/normal/unchecked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_mac/light/normal/unchecked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_mac/light/normal/unchecked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_mac/light/disabled/unchecked.svg")),
        };

        checkboxUncheckedIcons[1] = {
            QIcon(QStringLiteral(":/res/checkbox_mac/dark/normal/unchecked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_mac/dark/normal/unchecked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_mac/dark/normal/unchecked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_mac/dark/disabled/unchecked.svg")),
        };

        checkboxCheckedIcons[0] = {
            QIcon(QStringLiteral(":/res/checkbox_mac/light/normal/checked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_mac/light/normal/checked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_mac/light/normal/checked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_mac/light/disabled/checked.svg")),
        };

        checkboxCheckedIcons[1] = {
            QIcon(QStringLiteral(":/res/checkbox_mac/dark/normal/checked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_mac/dark/normal/checked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_mac/dark/normal/checked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_mac/dark/disabled/checked.svg")),
        };

        checkboxPartialIcons[0] = {
            QIcon(QStringLiteral(":/res/checkbox_mac/light/normal/partial.svg")),
            QIcon(QStringLiteral(":/res/checkbox_mac/light/normal/partial.svg")),
            QIcon(QStringLiteral(":/res/checkbox_mac/light/normal/partial.svg")),
            QIcon(QStringLiteral(":/res/checkbox_mac/light/disabled/partial.svg")),
        };

        checkboxPartialIcons[1] = {
            QIcon(QStringLiteral(":/res/checkbox_mac/dark/normal/partial.svg")),
            QIcon(QStringLiteral(":/res/checkbox_mac/dark/normal/partial.svg")),
            QIcon(QStringLiteral(":/res/checkbox_mac/dark/normal/partial.svg")),
            QIcon(QStringLiteral(":/res/checkbox_mac/dark/disabled/partial.svg")),
        };
#else
        checkboxUncheckedIcons[0] = {
            QIcon(QStringLiteral(":/res/checkbox_win/light/normal/unchecked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_win/light/hover/unchecked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_win/light/pressed/unchecked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_win/light/disabled/unchecked.svg")),
        };

        checkboxUncheckedIcons[1] = {
            QIcon(QStringLiteral(":/res/checkbox_win/dark/normal/unchecked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_win/dark/hover/unchecked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_win/dark/pressed/unchecked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_win/dark/disabled/unchecked.svg")),
        };

        checkboxCheckedIcons[0] = {
            QIcon(QStringLiteral(":/res/checkbox_win/light/normal/checked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_win/light/hover/checked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_win/light/pressed/checked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_win/light/disabled/checked.svg")),
        };

        checkboxCheckedIcons[1] = {
            QIcon(QStringLiteral(":/res/checkbox_win/dark/normal/checked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_win/dark/hover/checked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_win/dark/pressed/checked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_win/dark/disabled/checked.svg")),
        };

        checkboxPartialIcons[0] = {
            QIcon(QStringLiteral(":/res/checkbox_win/light/normal/partial.svg")),
            QIcon(QStringLiteral(":/res/checkbox_win/light/hover/partial.svg")),
            QIcon(QStringLiteral(":/res/checkbox_win/light/pressed/partial.svg")),
            QIcon(QStringLiteral(":/res/checkbox_win/light/disabled/partial.svg")),
        };

        checkboxPartialIcons[1] = {
            QIcon(QStringLiteral(":/res/checkbox_win/dark/normal/partial.svg")),
            QIcon(QStringLiteral(":/res/checkbox_win/dark/hover/partial.svg")),
            QIcon(QStringLiteral(":/res/checkbox_win/dark/pressed/partial.svg")),
            QIcon(QStringLiteral(":/res/checkbox_win/dark/disabled/partial.svg")),
        };
#endif

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
