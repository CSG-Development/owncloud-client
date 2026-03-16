/*
   This software is in the public domain, furnished "as is", without technical
   support, and with no warranty, express or implied, as to its usefulness for
   any purpose.
*/

#include <QtTest>
#include <QDebug>

#include "personalcloudpropagator_p.h"
#include "propagatedownload.h"
#include "qchar.h"

using namespace APP;
namespace APP {
QString APPLICATIONSYNC_EXPORT createDownloadTmpFileName(const QString &previous);
}

class TestPersonalCloudPropagator : public QObject
{
    Q_OBJECT

private slots:
    void testUpdateErrorFromSession()
    {
//        PersonalCloudPropagator propagator( NULL, QLatin1String("test1"), QLatin1String("test2"), new ProgressDatabase);
        QVERIFY( true );
    }

    void testTmpDownloadFileNameGeneration()
    {
        QString fn;
        // without dir
        for (int i = 1; i <= 1000; i++) {
            fn += QLatin1String("F");
            QString tmpFileName = createDownloadTmpFileName(fn);
            if (tmpFileName.contains(QLatin1Char('/'))) {
                tmpFileName = tmpFileName.mid(tmpFileName.lastIndexOf(QLatin1Char('/')) + 1);
            }
            QVERIFY( tmpFileName.length() > 0);
            QVERIFY( tmpFileName.length() <= 254);
        }
        // with absolute dir
        fn = QStringLiteral("/Users/guruz/ownCloud/rocks/GPL");
        for (int i = 1; i < 1000; i++) {
            fn += QLatin1Char('F');
            QString tmpFileName = createDownloadTmpFileName(fn);
            if (tmpFileName.contains(QLatin1Char('/'))) {
                tmpFileName = tmpFileName.mid(tmpFileName.lastIndexOf(QLatin1Char('/')) + 1);
            }
            QVERIFY( tmpFileName.length() > 0);
            QVERIFY( tmpFileName.length() <= 254);
        }
        // with relative dir
        fn = QStringLiteral("rocks/GPL");
        for (int i = 1; i < 1000; i++) {
            fn += QLatin1String("F");
            QString tmpFileName = createDownloadTmpFileName(fn);
            if (tmpFileName.contains(QLatin1Char('/'))) {
                tmpFileName = tmpFileName.mid(tmpFileName.lastIndexOf(QLatin1Char('/')) + 1);
            }
            QVERIFY( tmpFileName.length() > 0);
            QVERIFY( tmpFileName.length() <= 254);
        }
    }
};

QTEST_APPLESS_MAIN(TestPersonalCloudPropagator)
#include "testpersonalcloudpropagator.moc"
