/* Copyright C 2013 Klaralvdalens Datakonsult AB KDAB
 * Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef GENERICFACTORY_H
#define GENERICFACTORY_H

#include "kdtoolsglobal.h"

#include <QtCore/QHash>

template <typename BASE, typename IDENTIFIER = QString, typename... ARGUMENTS>
class GenericFactory
{
public:
    virtual ~GenericFactory() {}

    typedef BASE *(*FactoryFunction)(ARGUMENTS...);

    template <typename DERIVED>
    void registerProduct(const IDENTIFIER &id)
    {
        m_hash.insert(id, &GenericFactory::create<DERIVED>);
    }

    void registerProduct(const IDENTIFIER &id, FactoryFunction func)
    {
        m_hash.insert(id, func);
    }

    bool containsProduct(const IDENTIFIER &id) const
    {
        return m_hash.contains(id);
    }

    BASE *create(const IDENTIFIER &id, ARGUMENTS... args) const
    {
        const auto it = m_hash.constFind(id);
        if (it == m_hash.constEnd())
            return 0;
        return (*it)(std::forward<ARGUMENTS>(args)...);
    }

protected:
    GenericFactory() = default;

private:
    template <typename DERIVED>
    static BASE *create(ARGUMENTS... args)
    {
        return new DERIVED(std::forward<ARGUMENTS>(args)...);
    }

    GenericFactory(const GenericFactory &) = delete;
    GenericFactory &operator=(const GenericFactory &) = delete;

private:
    QHash<IDENTIFIER, FactoryFunction> m_hash;
};

#endif // GENERICFACTORY_H
