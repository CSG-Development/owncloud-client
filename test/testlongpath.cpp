/*
 * libcsync -- a library to sync a directory with another
 *
 * Copyright (c) 2013 by Klaas Freitag <freitag@owncloud.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#include "common/filesystembase.h"
#include "csync/csync.h"
#include "csync/vio/csync_vio_local.h"
#include "testutils/testutils.h"

#include <QTemporaryFile>
#include <QTest>


class TestLongWindowsPath : public QObject
{
    Q_OBJECT

private Q_SLOTS:
#ifdef Q_OS_WIN
    void check_long_win_path()
    {
        {
            const auto path = QStringLiteral("C://DATA/FILES/MUSIC/MY_MUSIC.mp3"); // check a short path
            const auto exp_path = QStringLiteral("\\\\?\\C:\\\\DATA\\FILES\\MUSIC\\MY_MUSIC.mp3");
            QString new_short = APP::FileSystem::longWinPath(path);
            QCOMPARE(new_short, exp_path);
        }

        {
            const auto path = QStringLiteral("\\\\foo\\bar/MY_MUSIC.mp3");
            const auto exp_path = QStringLiteral("\\\\foo\\bar\\MY_MUSIC.mp3");
            QString new_short = APP::FileSystem::longWinPath(path);
            QCOMPARE(new_short, exp_path);
        }

        {
            const auto path = QStringLiteral("//foo\\bar/MY_MUSIC.mp3");
            const auto exp_path = QStringLiteral("\\\\foo\\bar\\MY_MUSIC.mp3");
            QString new_short = APP::FileSystem::longWinPath(path);
            QCOMPARE(new_short, exp_path);
        }

        {
            const auto path = QStringLiteral("\\foo\\bar");
            const auto exp_path = QStringLiteral("\\\\?\\foo\\bar");
            QString new_short = APP::FileSystem::longWinPath(path);
            QCOMPARE(new_short, exp_path);
        }

        {
            const auto path = QStringLiteral("/foo/bar");
            const auto exp_path = QStringLiteral("\\\\?\\foo\\bar");
            QString new_short = APP::FileSystem::longWinPath(path);
            QCOMPARE(new_short, exp_path);
        }

        const auto longPath = QStringLiteral("D://alonglonglonglong/blonglonglonglong/clonglonglonglong/dlonglonglonglong/"
                                             "elonglonglonglong/flonglonglonglong/glonglonglonglong/hlonglonglonglong/ilonglonglonglong/"
                                             "jlonglonglonglong/klonglonglonglong/llonglonglonglong/mlonglonglonglong/nlonglonglonglong/"
                                             "olonglonglonglong/file.txt");
        const auto longPathConv = QStringLiteral("\\\\?\\D:\\\\alonglonglonglong\\blonglonglonglong\\clonglonglonglong\\dlonglonglonglong\\"
                                                 "elonglonglonglong\\flonglonglonglong\\glonglonglonglong\\hlonglonglonglong\\ilonglonglonglong\\"
                                                 "jlonglonglonglong\\klonglonglonglong\\llonglonglonglong\\mlonglonglonglong\\nlonglonglonglong\\"
                                                 "olonglonglonglong\\file.txt");

        QString new_long = APP::FileSystem::longWinPath(longPath);
        // printf( "XXXXXXXXXXXX %s %d\n", new_long, mem_reserved);

        QCOMPARE(new_long, longPathConv);

        // printf( "YYYYYYYYYYYY %ld\n", strlen(new_long));
        QCOMPARE(new_long.length(), 286);
    }
#endif


    void testLongPathStat_data()
    {
        QTest::addColumn<QString>("name");

        QTest::newRow("long") << QStringLiteral("/alonglonglonglong/blonglonglonglong/clonglonglonglong/dlonglonglonglong/"
                                                "elonglonglonglong/flonglonglonglong/glonglonglonglong/hlonglonglonglong/ilonglonglonglong/"
                                                "jlonglonglonglong/klonglonglonglong/llonglonglonglong/mlonglonglonglong/nlonglonglonglong/"
                                                "olonglonglonglong/file.txt");
        QTest::newRow("long emoji") << QString::fromUtf8("/alonglonglonglong/blonglonglonglong/clonglonglonglong/dlonglonglonglong/"
                                                         "elonglonglonglong/flonglonglonglong/glonglonglonglong/hlonglonglonglong/ilonglonglonglong/"
                                                         "jlonglonglonglong/klonglonglonglong/llonglonglonglong/mlonglonglonglong/nlonglonglonglong/"
                                                         "olonglonglonglong/file🐷.txt");
        QTest::newRow("long russian") << QString::fromUtf8("/alonglonglonglong/blonglonglonglong/clonglonglonglong/dlonglonglonglong/"
                                                           "elonglonglonglong/flonglonglonglong/glonglonglonglong/hlonglonglonglong/ilonglonglonglong/"
                                                           "jlonglonglonglong/klonglonglonglong/llonglonglonglong/mlonglonglonglong/nlonglonglonglong/"
                                                           "olonglonglonglong/собственное.txt");
        QTest::newRow("long arabic") << QString::fromUtf8("/alonglonglonglong/blonglonglonglong/clonglonglonglong/dlonglonglonglong/"
                                                          "elonglonglonglong/flonglonglonglong/glonglonglonglong/hlonglonglonglong/ilonglonglonglong/"
                                                          "jlonglonglonglong/klonglonglonglong/llonglonglonglong/mlonglonglonglong/nlonglonglonglong/"
                                                          "olonglonglonglong/السحاب.txt");
        QTest::newRow("long chinese") << QString::fromUtf8("/alonglonglonglong/blonglonglonglong/clonglonglonglong/dlonglonglonglong/"
                                                           "elonglonglonglong/flonglonglonglong/glonglonglonglong/hlonglonglonglong/ilonglonglonglong/"
                                                           "jlonglonglonglong/klonglonglonglong/llonglonglonglong/mlonglonglonglong/nlonglonglonglong/"
                                                           "olonglonglonglong/自己的云.txt");
    }

    void testLongPathStat()
    {
        auto tmp = APP::TestUtils::createTempDir();
        QFETCH(QString, name);
        const QFileInfo longPath(tmp.path() + name);

        const auto data = QByteArrayLiteral("hello");
        qDebug() << longPath;
        QVERIFY(longPath.dir().mkpath(QStringLiteral(".")));

        QFile file(longPath.filePath());
        QVERIFY(file.open(QFile::WriteOnly));
        QVERIFY(file.write(data.constData()) == data.size());
        file.close();

        csync_file_stat_t buf;
        QVERIFY(csync_vio_local_stat(longPath.filePath(), &buf) != -1);
        QVERIFY(buf.size == data.size());
        QVERIFY(buf.size == longPath.size());

        QVERIFY(tmp.remove());
    }
};

QTEST_GUILESS_MAIN(TestLongWindowsPath)
#include "testlongpath.moc"
