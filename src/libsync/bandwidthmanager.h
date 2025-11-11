/*
 * Copyright (C) by Markus Goetz <markus@woboq.com>
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

#ifndef BANDWIDTHMANAGER_H
#define BANDWIDTHMANAGER_H

#include <QObject>
#include <QTimer>
#include <QIODevice>

#include <list>

namespace CUR {

class UploadDevice;
class GETFileJob;
class CuratorPropagator;

/**
 * @brief The BandwidthManager class
 * @ingroup libsync
 */
class BandwidthManager : public QObject
{
    Q_OBJECT
public:
    BandwidthManager(CuratorPropagator *p);
    ~BandwidthManager() override;

    bool usingAbsoluteUploadLimit() { return _currentUploadLimit > 0; }
    bool usingRelativeUploadLimit() { return _currentUploadLimit < 0; }
    bool usingAbsoluteDownloadLimit() { return _currentDownloadLimit > 0; }
    bool usingRelativeDownloadLimit() { return _currentDownloadLimit < 0; }


    qint64 currentDownloadLimit() const;
    void setCurrentDownloadLimit(qint64 newCurrentDownloadLimit);

    qint64 currentUploadLimit() const;
    void setCurrentUploadLimit(qint64 newCurrentUploadLimit);

public slots:
    void registerUploadDevice(UploadDevice *);
    void unregisterUploadDevice(QObject *);

    void registerDownloadJob(GETFileJob *);
    void unregisterDownloadJob(GETFileJob *);

    void absoluteLimitTimerExpired();

    void relativeUploadMeasuringTimerExpired();
    void relativeUploadDelayTimerExpired();

    void relativeDownloadMeasuringTimerExpired();
    void relativeDownloadDelayTimerExpired();

private:
    // FIXME this timer and this variable should be replaced
    // by the propagator emitting the changed limit values to us as signal
    CuratorPropagator *_propagator = nullptr;

    // for absolute up/down bw limiting
    QTimer _absoluteLimitTimer;

    // FIXME merge these two lists
    std::list<UploadDevice *> _absoluteUploadDeviceList;
    std::list<UploadDevice *> _relativeUploadDeviceList;

    QTimer _relativeUploadMeasuringTimer;

    // for relative bw limiting, we need to wait this amount before measuring again
    QTimer _relativeUploadDelayTimer;

    // the device measured
    UploadDevice *_relativeLimitCurrentMeasuredDevice = nullptr;

    // for measuring how much progress we made at start
    qint64 _relativeUploadLimitProgressAtMeasuringRestart = 0;
    qint64 _currentUploadLimit = 0;

    std::list<GETFileJob *> _downloadJobList;
    QTimer _relativeDownloadMeasuringTimer;

    // for relative bw limiting, we need to wait this amount before measuring again
    QTimer _relativeDownloadDelayTimer;

    // the device measured
    GETFileJob *_relativeLimitCurrentMeasuredJob = nullptr;

    // for measuring how much progress we made at start
    qint64 _relativeDownloadLimitProgressAtMeasuringRestart = 0;

    qint64 _currentDownloadLimit = 0;
};
}

#endif
