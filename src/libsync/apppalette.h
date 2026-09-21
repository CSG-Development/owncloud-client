#pragma once

#include "personalcloudlib.h"

#include <QColor>
#include <QStringView>
#include <optional>

namespace APP {

enum class ColorToken {
    Primary,
    PrimaryHover,
    PrimaryPressed,
    PrimaryDisabled,
    OnPrimary,
    OnPrimaryDisabled,
    LinkText,
    IconButton,
    IconButtonHover,
    IconButtonPressed,
    IconButtonDisabled,
    ToolButtonFocusRing,
    PushButtonFocusRing,
    DialogFocusFrame,
    TabCheckedBackground,
    TabCheckedHoverBackground,
    ToolButtonCheckedBackground,
    ToolButtonCheckedHoverBackground,
    SelectionBackground,
    ProgressFill,
    ProgressTrack,
    SpinnerTrack,
    MacAccentTopHighlight,
    MacAccentGradientTop,
    MacAccentGradientBottom,
    OnMacAccent,
    WizardHeaderBackground,
    OnWizardHeader,
    OutlineButton,
    FlatButtonHoverBackground,
    FlatButtonPressedBackground,
    ComboArrowHoverBackground,
    ComboPopupSelectionBackground,
    MenuSelectionBackground,
    TabSelectedBackground,
    InputFocusBorder,
    DialogOnPrimary,
    DialogOnPrimaryPressed,
    ButtonFrameFocused,
    ButtonFrameNormal,
    ButtonFramePressed,
    ButtonFrameHovered,
    ButtonFrameDisabled,
    ButtonBackgroundNormal,
    ButtonBackgroundPressed,
    ButtonBackgroundHovered,
    ButtonBackgroundDisabled,
    ArrowButtonBackgroundDisabled,
    MacButtonFrameNormal,
    MacButtonFramePressed,
    MacButtonFrameHovered,
    MacButtonFrameDisabled,
    MacArrowButtonFrameDisabled,
    MacButtonBackgroundNormal,
    MacArrowButtonBackgroundNormal,
    MacButtonBackgroundPressed,
    MacButtonBackgroundHovered,
    MacButtonBackgroundDisabled,
    ToolButtonBackgroundNormal,
    ToolButtonBackgroundPressed,
    ToolButtonBackgroundHovered,
    ToolButtonBackgroundDisabled,
    MacStandardTopHighlight,
    MacStandardBackgroundNormal,
    MacStandardBackgroundPressed,
    MacStandardText,
    TextButton,
    TextButtonHover,
    TextButtonPressed,
    TextButtonDisabled,
    OutlineButtonHover,
    OutlineButtonPressed,
    OutlineButtonHoverBackground,
    OutlineButtonPressedBackground,
    OutlineButtonDisabled,
    ToolbarButtonHoverBackground,
    ToolbarButtonPressedBackground,
    WindowBackground,
    WizardCardBackground,
    ToolbarBackground,
    ToolbarBorder,
    OnMenuSelection,
    OnTabSelected
};

class APPLICATIONSYNC_EXPORT AppPalette
{
public:
    static QColor light(ColorToken token);
    static QColor dark(ColorToken token);
    static QColor color(ColorToken token, bool isDark);
    static std::optional<ColorToken> tokenFromName(QStringView name);
};

} // namespace APP
