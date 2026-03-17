#pragma once

#include <QMetaProperty>

template <typename T>
bool safeSetProperty(QObject* obj, const char* name, const T& value) {
    if (!obj)
        return false;

    const QMetaObject* meta = obj->metaObject();
    int index = meta->indexOfProperty(name);

    if (index == -1) {
        return false;
    }

    QMetaProperty prop = meta->property(index);
    if (prop.isWritable()) {
        return prop.write(obj, QVariant::fromValue(value));
    }

    return false;
}

