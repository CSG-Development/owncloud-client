#pragma once

#include <QIcon>
#include <QStyle>

namespace RadiobuttonRes {

void init();

const QIcon& getRbIcon(QStyle::State state, bool isDark);

} // namespace CheckboxRes
