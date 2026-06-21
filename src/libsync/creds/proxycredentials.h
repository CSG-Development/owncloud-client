/*
 * Copyright (C) by the PersonalCloud developers
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#pragma once

#include "personalcloudlib.h"

#include <QObject>

#include <functional>

namespace QKeychain
{
class Job;
}

namespace APP
{

class CredentialManager;

class APPLICATIONSYNC_EXPORT ProxyCredentials : public QObject
{
    Q_OBJECT
public:
    static QString passwordKey();

    static void load(QObject *context, const std::function<void(const QString &password)> &callback);
    static void store(const QString &password);
    static void remove();
    static void removeLegacyPassword();

private:
    explicit ProxyCredentials(QObject *parent);

    QKeychain::Job *startLegacyMigration(QString *password);

    CredentialManager *_manager;
};

}   // namespace APP
