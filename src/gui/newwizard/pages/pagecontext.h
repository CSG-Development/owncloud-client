#pragma once

#include "device/devicetypes.h"
#include <QMetaType>

struct PageContext {
    Q_GADGET

public:
    QString errorString;
    bool emailErrorState = false;

    bool operator==(const PageContext& other) const {
        return std::tie(errorString, emailErrorState) ==
               std::tie(other.errorString, other.emailErrorState);
    }
    bool operator!=(const PageContext& other) const {
        return !(*this == other);
    }
};

struct CredentialsContext {
    std::optional<Device> device;
    QString email;
    QString password;
};

Q_DECLARE_METATYPE(PageContext)
