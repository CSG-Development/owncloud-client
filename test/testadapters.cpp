/*
 * Copyright (C) Erik Verbruggen <erik.verbruggenkiteworks.com>
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

#include <QtTest>

#include "../src/gui/networkadapters/discoverwebfingerserviceadapter.h"
#include "testutils/syncenginetestutils.h"

using namespace OCC;

class TestAdapters : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testWebFingerDiscoveryFailure()
    {
        FakeAM fakeAM({}, nullptr);
        fakeAM.setOverride([&fakeAM](QNetworkAccessManager::Operation op, const QNetworkRequest &req, QIODevice *device) -> QNetworkReply * {
            Q_UNUSED(device);
            // no well-known endpoints defined
            return new FakeErrorReply(op, req, &fakeAM, 404);
        });

        DiscoverWebFingerServiceAdapter adapter(&fakeAM, QUrl::fromUserInput(QStringLiteral("file:///")));
        DiscoverWebFingerServiceResult result = adapter.getResult();
        QVERIFY(!result.error.isEmpty());
        QVERIFY(result.href.isEmpty());
    }
};

QTEST_GUILESS_MAIN(TestAdapters)
#include "testadapters.moc"
