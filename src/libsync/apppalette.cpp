#include "apppalette.h"

namespace APP {

namespace {

struct TokenColors
{
    QColor light;
    QColor dark;
};

TokenColors colorsFor(ColorToken token)
{
    switch (token) {
    case ColorToken::Primary:                          return {QColor(0x6E, 0xBE, 0x49), QColor(0x6E, 0xBE, 0x49)};
    case ColorToken::PrimaryHover:                     return {QColor(0x8C, 0xD8, 0x73), QColor(0x8C, 0xD8, 0x73)};
    case ColorToken::PrimaryPressed:                   return {QColor(0xA9, 0xE1, 0x96), QColor(0xA9, 0xE1, 0x96)};
    case ColorToken::PrimaryDisabled:                  return {QColor(0x19, 0x19, 0x19, 0x33), QColor(0xF6, 0xF6, 0xF6, 0x4C)};
    case ColorToken::OnPrimary:                        return {QColor(0x00, 0x00, 0x00, 0xDE), QColor(0x00, 0x00, 0x00, 0xDE)};
    case ColorToken::OnPrimaryDisabled:                return {QColor(0x6D, 0x6D, 0x6D), QColor(0x3D, 0x3D, 0x3D)};
    case ColorToken::LinkText:                         return {QColor(0x1F, 0x54, 0x2F), QColor(0xA9, 0xE1, 0x96)};
    case ColorToken::IconButton:                       return {QColor(0x2F, 0x74, 0x3C), QColor(0x8C, 0xD8, 0x73)};
    case ColorToken::IconButtonHover:                  return {QColor(0x8C, 0xD8, 0x73), QColor(0x8C, 0xD8, 0x73)};
    case ColorToken::IconButtonPressed:                return {QColor(0xA9, 0xE1, 0x96), QColor(0xA9, 0xE1, 0x96)};
    case ColorToken::IconButtonDisabled:               return {QColor(0xBF, 0xBF, 0xBF), QColor(0x7E, 0x7E, 0x7E)};
    case ColorToken::ToolButtonFocusRing:              return {QColor(0x6E, 0xBE, 0x49, 0x80), QColor(0x6E, 0xBE, 0x49, 0x80)};
    case ColorToken::PushButtonFocusRing:              return {QColor(0x6E, 0xBE, 0x49, 0x80), QColor(0x6E, 0xBE, 0x49, 0x80)};
    case ColorToken::DialogFocusFrame:                 return {QColor(0x6E, 0xBE, 0x49, 0x7F), QColor(0xDE, 0xDE, 0xDE, 0xFF)};
    case ColorToken::TabCheckedBackground:             return {QColor(0x2F, 0x74, 0x3C, 0x33), QColor(0x8C, 0xD8, 0x73, 0x33)};
    case ColorToken::TabCheckedHoverBackground:        return {QColor(0x2F, 0x74, 0x3C, 0x33), QColor(0x8C, 0xD8, 0x73, 0x33)};
    case ColorToken::ToolButtonCheckedBackground:      return {QColor(0x2F, 0x74, 0x3C, 0x33), QColor(0x8C, 0xD8, 0x73, 0x33)};
    case ColorToken::ToolButtonCheckedHoverBackground: return {QColor(0x2F, 0x74, 0x3C, 0x33), QColor(0x8C, 0xD8, 0x73, 0x33)};
    case ColorToken::SelectionBackground:              return {QColor(0x2F, 0x74, 0x3C, 0x3D), QColor(0x8C, 0xD8, 0x73, 0x3D)};
    case ColorToken::ProgressFill:                     return {QColor(0x2F, 0x74, 0x3C), QColor(0x8C, 0xD8, 0x73)};
    case ColorToken::ProgressTrack:                    return {QColor(0x00, 0x00, 0x00, 0x26), QColor(0xFF, 0xFF, 0xFF, 0x33)};
    case ColorToken::SpinnerTrack:                     return {QColor(0xE0, 0xE0, 0xE0), QColor(0x61, 0x61, 0x61)};
    case ColorToken::MacAccentTopHighlight:            return {QColor(0xFF, 0xFF, 0xFF, 0x59), QColor(0xFF, 0xFF, 0xFF, 0x59)};
    case ColorToken::MacAccentGradientTop:             return {QColor(0x87, 0xC9, 0x68), QColor(0x87, 0xC9, 0x68)};
    case ColorToken::MacAccentGradientBottom:          return {QColor(0x6A, 0xB8, 0x47), QColor(0x6A, 0xB8, 0x47)};
    case ColorToken::OnMacAccent:                      return {QColor(0x00, 0x00, 0x00, 0xDE), QColor(0x00, 0x00, 0x00, 0xDE)};
    case ColorToken::WizardHeaderBackground:           return {QColor(0x04, 0x1E, 0x42), QColor(0x04, 0x1E, 0x42)};
    case ColorToken::OnWizardHeader:                   return {QColor(0xFF, 0xFF, 0xFF), QColor(0xFF, 0xFF, 0xFF)};
    case ColorToken::OutlineButton:                    return {QColor(0x2F, 0x74, 0x3C), QColor(0x6E, 0xBE, 0x49)};
    case ColorToken::FlatButtonHoverBackground:        return {QColor(0x2F, 0x74, 0x3C, 0x19), QColor(0x55, 0xA7, 0x2F, 0x19)};
    case ColorToken::FlatButtonPressedBackground:      return {QColor(0x2F, 0x74, 0x3C, 0x33), QColor(0x55, 0xA7, 0x2F, 0x33)};
    case ColorToken::ComboArrowHoverBackground:        return {QColor(0x2F, 0x74, 0x3C, 0x3D), QColor(0x8C, 0xD8, 0x73, 0x1E)};
    case ColorToken::ComboPopupSelectionBackground:    return {QColor(0x2F, 0x74, 0x3C, 0x33), QColor(0x8C, 0xD8, 0x73, 0x3D)};
    case ColorToken::MenuSelectionBackground:          return {QColor(0x6E, 0xBE, 0x49), QColor(0x6E, 0xBE, 0x49)};
    case ColorToken::TabSelectedBackground:            return {QColor(0x2F, 0x74, 0x3C), QColor(0x8C, 0xD8, 0x73)};
    case ColorToken::InputFocusBorder:                 return {QColor(0x6E, 0xBE, 0x49), QColor(0x6E, 0xBE, 0x49)};
    case ColorToken::DialogOnPrimary:                  return {QColor(0x00, 0x00, 0x00, 0xDE), QColor(0x00, 0x00, 0x00, 0xDE)};
    case ColorToken::DialogOnPrimaryPressed:           return {QColor(0x00, 0x00, 0x00, 0xB2), QColor(0x00, 0x00, 0x00, 0xB2)};
    case ColorToken::ButtonFrameFocused:               return {QColor(0x21, 0x21, 0x21), QColor(0xFF, 0xFF, 0xFF)};
    case ColorToken::ButtonFrameNormal:                return {QColor(0xCB, 0xCD, 0xD3), QColor(0xFF, 0xFF, 0xFF, 0x18)};
    case ColorToken::ButtonFramePressed:               return {QColor(0xCB, 0xCD, 0xD3), QColor(0xFF, 0xFF, 0xFF, 0x12)};
    case ColorToken::ButtonFrameHovered:               return {QColor(0xCB, 0xCD, 0xD3), QColor(0xFF, 0xFF, 0xFF, 0x18)};
    case ColorToken::ButtonFrameDisabled:              return {QColor(0xCB, 0xCD, 0xD3), QColor(0xFF, 0xFF, 0xFF, 0x12)};
    case ColorToken::ButtonBackgroundNormal:           return {QColor(0xFF, 0xFF, 0xFF, 0xB2), QColor(0xFF, 0xFF, 0xFF, 0x0F)};
    case ColorToken::ButtonBackgroundPressed:          return {QColor(0x61, 0x61, 0x61, 0x08), QColor(0xFF, 0xFF, 0xFF, 0x08)};
    case ColorToken::ButtonBackgroundHovered:          return {QColor(0x61, 0x61, 0x61, 0x1F), QColor(0xFF, 0xFF, 0xFF, 0x15)};
    case ColorToken::ButtonBackgroundDisabled:         return {QColor(0xF6, 0xF6, 0xF6), QColor(0x00, 0x00, 0x00, 0x00)};
    case ColorToken::ArrowButtonBackgroundDisabled:    return {QColor(0xF6, 0xF6, 0xF6), QColor(0xFF, 0xFF, 0xFF, 0x0B)};
    case ColorToken::MacButtonFrameNormal:             return {QColor(0xCB, 0xCD, 0xD3), QColor(0x61, 0x61, 0x61)};
    case ColorToken::MacButtonFramePressed:            return {QColor(0xCB, 0xCD, 0xD3), QColor(0x61, 0x61, 0x61)};
    case ColorToken::MacButtonFrameHovered:            return {QColor(0xCB, 0xCD, 0xD3), QColor(0x61, 0x61, 0x61)};
    case ColorToken::MacButtonFrameDisabled:           return {QColor(0xBB, 0xBB, 0xBB), QColor(0x6B, 0x6C, 0x6D)};
    case ColorToken::MacArrowButtonFrameDisabled:      return {QColor(0xCB, 0xCD, 0xD3), QColor(0x26, 0x27, 0x29)};
    case ColorToken::MacButtonBackgroundNormal:        return {QColor(0xFF, 0xFF, 0xFF), QColor(0x61, 0x61, 0x61)};
    case ColorToken::MacArrowButtonBackgroundNormal:   return {QColor(0xFF, 0xFF, 0xFF, 0xB2), QColor(0x61, 0x61, 0x61, 0xB2)};
    case ColorToken::MacButtonBackgroundPressed:       return {QColor(0x61, 0x61, 0x61, 0x08), QColor(0xFF, 0xFF, 0xFF, 0x1F)};
    case ColorToken::MacButtonBackgroundHovered:       return {QColor(0x61, 0x61, 0x61, 0x1F), QColor(0xFF, 0xFF, 0xFF, 0x08)};
    case ColorToken::MacButtonBackgroundDisabled:      return {QColor(0xF6, 0xF6, 0xF6), QColor(0x44, 0x45, 0x46)};
    case ColorToken::ToolButtonBackgroundNormal:       return {QColor(0x00, 0x00, 0x00, 0x00), QColor(0x00, 0x00, 0x00, 0x00)};
    case ColorToken::ToolButtonBackgroundPressed:      return {QColor(0xF5, 0xF5, 0xF7), QColor(0x4E, 0x50, 0x53)};
    case ColorToken::ToolButtonBackgroundHovered:      return {QColor(0xE6, 0xE3, 0xE6), QColor(0xFF, 0xFF, 0xFF, 0x1A)};
    case ColorToken::ToolButtonBackgroundDisabled:     return {QColor(0x00, 0x00, 0x00, 0x00), QColor(0x00, 0x00, 0x00, 0x00)};
    case ColorToken::MacStandardTopHighlight:          return {QColor(0xFF, 0xFF, 0xFF, 0xC8), QColor(0xFF, 0xFF, 0xFF, 0x19)};
    case ColorToken::MacStandardBackgroundNormal:      return {QColor(0xE6, 0xE6, 0xEB), QColor(0x3C, 0x3C, 0x3E)};
    case ColorToken::MacStandardBackgroundPressed:     return {QColor(0xD7, 0xD7, 0xDC), QColor(0x46, 0x46, 0x48)};
    case ColorToken::MacStandardText:                  return {QColor(0x1E, 0x1E, 0x1E), QColor(0xE6, 0xE6, 0xE6)};
    case ColorToken::TextButton:                       return {QColor(0x2F, 0x74, 0x3C), QColor(0x6E, 0xBE, 0x49)};
    case ColorToken::TextButtonHover:                  return {QColor(0x27, 0x8A, 0x2C), QColor(0x8C, 0xD8, 0x73)};
    case ColorToken::TextButtonPressed:                return {QColor(0x55, 0xA7, 0x2F), QColor(0xA9, 0xE1, 0x96)};
    case ColorToken::TextButtonDisabled:               return {QColor(0x19, 0x19, 0x19, 0x33), QColor(0xF6, 0xF6, 0xF6, 0x4C)};
    case ColorToken::OutlineButtonHover:               return {QColor(0x27, 0x8A, 0x2C), QColor(0x8C, 0xD8, 0x73)};
    case ColorToken::OutlineButtonPressed:             return {QColor(0x55, 0xA7, 0x2F), QColor(0xA9, 0xE1, 0x96)};
    case ColorToken::OutlineButtonHoverBackground:     return {QColor(0x8C, 0xD8, 0x73, 0x33), QColor(0x8C, 0xD8, 0x73, 0x33)};
    case ColorToken::OutlineButtonPressedBackground:   return {QColor(0x55, 0xA7, 0x2F, 0x33), QColor(0x55, 0xA7, 0x2F, 0x33)};
    case ColorToken::OutlineButtonDisabled:            return {QColor(0x19, 0x19, 0x19, 0x33), QColor(0xF6, 0xF6, 0xF6, 0x4C)};
    case ColorToken::ToolbarButtonHoverBackground:     return {QColor(0x2F, 0x74, 0x3C, 0x19), QColor(0x8C, 0xD8, 0x73, 0x19)};
    case ColorToken::ToolbarButtonPressedBackground:   return {QColor(0x2F, 0x74, 0x3C, 0x33), QColor(0x8C, 0xD8, 0x73, 0x33)};
    case ColorToken::WindowBackground:                 return {QColor(0xE7, 0xE7, 0xE7), QColor(0x4F, 0x4F, 0x4F)};
    case ColorToken::WizardCardBackground:             return {QColor(0xF6, 0xF6, 0xF6), QColor(0x19, 0x19, 0x19)};
    case ColorToken::ToolbarBackground:                return {QColor(0xF6, 0xF6, 0xF6), QColor(0x19, 0x19, 0x19)};
    case ColorToken::ToolbarBorder:                    return {QColor(0xD1, 0xD1, 0xD1), QColor(0x5D, 0x5D, 0x5D)};
    case ColorToken::OnMenuSelection:                  return {QColor(0x00, 0x00, 0x00, 0xDE), QColor(0x00, 0x00, 0x00, 0xDE)};
    case ColorToken::OnTabSelected:                    return {QColor(0xFF, 0xFF, 0xFF, 0xDE), QColor(0x00, 0x00, 0x00, 0xDE)};
    case ColorToken::TreeItemText:                     return {QColor(0x19, 0x19, 0x19), QColor(0xF6, 0xF6, 0xF6)};
    case ColorToken::TreeItemHover:                    return {QColor(0x00, 0x00, 0x00, 0x0A), QColor(0xFF, 0xFF, 0xFF, 0x0A)};
    case ColorToken::TreeItemSelected:                 return {QColor(0x00, 0x00, 0x00, 0x06), QColor(0x00, 0x00, 0x00, 0x1A)};
    }

    return {};
}

struct TokenName
{
    const char *name;
    ColorToken token;
};

constexpr TokenName tokenNames[] = {
    {"primary", ColorToken::Primary},
    {"primaryHover", ColorToken::PrimaryHover},
    {"primaryPressed", ColorToken::PrimaryPressed},
    {"primaryDisabled", ColorToken::PrimaryDisabled},
    {"onPrimary", ColorToken::OnPrimary},
    {"onPrimaryDisabled", ColorToken::OnPrimaryDisabled},
    {"linkText", ColorToken::LinkText},
    {"iconButton", ColorToken::IconButton},
    {"iconButtonHover", ColorToken::IconButtonHover},
    {"iconButtonPressed", ColorToken::IconButtonPressed},
    {"iconButtonDisabled", ColorToken::IconButtonDisabled},
    {"toolButtonFocusRing", ColorToken::ToolButtonFocusRing},
    {"pushButtonFocusRing", ColorToken::PushButtonFocusRing},
    {"dialogFocusFrame", ColorToken::DialogFocusFrame},
    {"tabCheckedBackground", ColorToken::TabCheckedBackground},
    {"tabCheckedHoverBackground", ColorToken::TabCheckedHoverBackground},
    {"toolButtonCheckedBackground", ColorToken::ToolButtonCheckedBackground},
    {"toolButtonCheckedHoverBackground", ColorToken::ToolButtonCheckedHoverBackground},
    {"selectionBackground", ColorToken::SelectionBackground},
    {"progressFill", ColorToken::ProgressFill},
    {"progressTrack", ColorToken::ProgressTrack},
    {"spinnerTrack", ColorToken::SpinnerTrack},
    {"macAccentTopHighlight", ColorToken::MacAccentTopHighlight},
    {"macAccentGradientTop", ColorToken::MacAccentGradientTop},
    {"macAccentGradientBottom", ColorToken::MacAccentGradientBottom},
    {"onMacAccent", ColorToken::OnMacAccent},
    {"wizardHeaderBackground", ColorToken::WizardHeaderBackground},
    {"onWizardHeader", ColorToken::OnWizardHeader},
    {"outlineButton", ColorToken::OutlineButton},
    {"flatButtonHoverBackground", ColorToken::FlatButtonHoverBackground},
    {"flatButtonPressedBackground", ColorToken::FlatButtonPressedBackground},
    {"comboArrowHoverBackground", ColorToken::ComboArrowHoverBackground},
    {"comboPopupSelectionBackground", ColorToken::ComboPopupSelectionBackground},
    {"menuSelectionBackground", ColorToken::MenuSelectionBackground},
    {"tabSelectedBackground", ColorToken::TabSelectedBackground},
    {"inputFocusBorder", ColorToken::InputFocusBorder},
    {"dialogOnPrimary", ColorToken::DialogOnPrimary},
    {"dialogOnPrimaryPressed", ColorToken::DialogOnPrimaryPressed},
    {"buttonFrameFocused", ColorToken::ButtonFrameFocused},
    {"buttonFrameNormal", ColorToken::ButtonFrameNormal},
    {"buttonFramePressed", ColorToken::ButtonFramePressed},
    {"buttonFrameHovered", ColorToken::ButtonFrameHovered},
    {"buttonFrameDisabled", ColorToken::ButtonFrameDisabled},
    {"buttonBackgroundNormal", ColorToken::ButtonBackgroundNormal},
    {"buttonBackgroundPressed", ColorToken::ButtonBackgroundPressed},
    {"buttonBackgroundHovered", ColorToken::ButtonBackgroundHovered},
    {"buttonBackgroundDisabled", ColorToken::ButtonBackgroundDisabled},
    {"arrowButtonBackgroundDisabled", ColorToken::ArrowButtonBackgroundDisabled},
    {"macButtonFrameNormal", ColorToken::MacButtonFrameNormal},
    {"macButtonFramePressed", ColorToken::MacButtonFramePressed},
    {"macButtonFrameHovered", ColorToken::MacButtonFrameHovered},
    {"macButtonFrameDisabled", ColorToken::MacButtonFrameDisabled},
    {"macArrowButtonFrameDisabled", ColorToken::MacArrowButtonFrameDisabled},
    {"macButtonBackgroundNormal", ColorToken::MacButtonBackgroundNormal},
    {"macArrowButtonBackgroundNormal", ColorToken::MacArrowButtonBackgroundNormal},
    {"macButtonBackgroundPressed", ColorToken::MacButtonBackgroundPressed},
    {"macButtonBackgroundHovered", ColorToken::MacButtonBackgroundHovered},
    {"macButtonBackgroundDisabled", ColorToken::MacButtonBackgroundDisabled},
    {"toolButtonBackgroundNormal", ColorToken::ToolButtonBackgroundNormal},
    {"toolButtonBackgroundPressed", ColorToken::ToolButtonBackgroundPressed},
    {"toolButtonBackgroundHovered", ColorToken::ToolButtonBackgroundHovered},
    {"toolButtonBackgroundDisabled", ColorToken::ToolButtonBackgroundDisabled},
    {"macStandardTopHighlight", ColorToken::MacStandardTopHighlight},
    {"macStandardBackgroundNormal", ColorToken::MacStandardBackgroundNormal},
    {"macStandardBackgroundPressed", ColorToken::MacStandardBackgroundPressed},
    {"macStandardText", ColorToken::MacStandardText},
    {"textButton", ColorToken::TextButton},
    {"textButtonHover", ColorToken::TextButtonHover},
    {"textButtonPressed", ColorToken::TextButtonPressed},
    {"textButtonDisabled", ColorToken::TextButtonDisabled},
    {"outlineButtonHover", ColorToken::OutlineButtonHover},
    {"outlineButtonPressed", ColorToken::OutlineButtonPressed},
    {"outlineButtonHoverBackground", ColorToken::OutlineButtonHoverBackground},
    {"outlineButtonPressedBackground", ColorToken::OutlineButtonPressedBackground},
    {"outlineButtonDisabled", ColorToken::OutlineButtonDisabled},
    {"toolbarButtonHoverBackground", ColorToken::ToolbarButtonHoverBackground},
    {"toolbarButtonPressedBackground", ColorToken::ToolbarButtonPressedBackground},
    {"windowBackground", ColorToken::WindowBackground},
    {"wizardCardBackground", ColorToken::WizardCardBackground},
    {"toolbarBackground", ColorToken::ToolbarBackground},
    {"toolbarBorder", ColorToken::ToolbarBorder},
    {"onMenuSelection", ColorToken::OnMenuSelection},
    {"onTabSelected", ColorToken::OnTabSelected},
    {"treeItemText", ColorToken::TreeItemText},
    {"treeItemHover", ColorToken::TreeItemHover},
    {"treeItemSelected", ColorToken::TreeItemSelected},
};

static_assert(sizeof(tokenNames) / sizeof(tokenNames[0]) == static_cast<size_t>(ColorToken::TreeItemSelected) + 1,
              "tokenNames must list every ColorToken");

} // namespace

QColor AppPalette::light(ColorToken token)
{
    return colorsFor(token).light;
}

QColor AppPalette::dark(ColorToken token)
{
    return colorsFor(token).dark;
}

QColor AppPalette::color(ColorToken token, bool isDark)
{
    return isDark ? dark(token) : light(token);
}

std::optional<ColorToken> AppPalette::tokenFromName(QStringView name)
{
    for (const TokenName &entry : tokenNames) {
        if (name == QLatin1StringView(entry.name))
            return entry.token;
    }

    return std::nullopt;
}

} // namespace APP
