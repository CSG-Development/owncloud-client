/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef PROGRESSCOORDINATOR_H
#define PROGRESSCOORDINATOR_H

#include "installer_global.h"

#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QPointer>

namespace QInstaller {

class INSTALLER_EXPORT ProgressCoordinator : public QObject
{
    Q_OBJECT

    struct ProgressSpinner {
        ProgressSpinner()
            : spinnerChars(QLatin1String("/-\\|"))
            , currentIndex(0) {}

        const QString spinnerChars;
        quint8 currentIndex;
    };

public:
    static ProgressCoordinator *instance();
    ~ProgressCoordinator();

    void registerPartProgress(QObject *sender, const char *signal, double partProgressSize);

public slots:
    void reset();
    void setUndoMode();

    QString labelText() const;
    void setLabelText(const QString &text);

    int progressInPercentage() const;
    void partProgressChanged(double fraction);

    void addManualPercentagePoints(int value);
    void addReservePercentagePoints(int value);

    void emitDetailTextChanged(const QString &text);
    void emitLabelAndDetailTextChanged(const QString &text);

    void emitAdditionalProgressStatus(const QString &status);
    void printProgressPercentage(int progress);
    void printProgressMessage(const QString &message);

signals:
    void detailTextChanged(const QString &text);
    void detailTextResetNeeded();
    void additionalProgressStatusChanged(const QString &status);

protected:
    explicit ProgressCoordinator(QObject *parent);

private:
    double allPendingCalculatedPartPercentages(const QString &excludeKeyObject = QString());
    void disconnectAllSenders();

private:
    QHash<QString, double> m_senderPendingCalculatedPercentageHash;
    QHash<QString, double> m_senderPartProgressSizeHash;
    ProgressSpinner *m_progressSpinner;
    QString m_installationLabelText;
    double m_currentCompletePercentage;
    double m_currentBasePercentage;
    int m_manualAddedPercentage;
    int m_reservedPercentage;
    bool m_undoMode;
    double m_reachedPercentageBeforeUndo;
};

} //namespace QInstaller

#endif //PROGRESSCOORDINATOR_H
