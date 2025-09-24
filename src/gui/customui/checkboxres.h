#pragma once

#include <QIcon>
#include <QStyle>

namespace CheckboxRes {

void init();

const QIcon& getChkIcon(QStyle::State state, bool isDark);

} // namespace CheckboxRes
