/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef PERFORMINSTALLATIONFORM_H
#define PERFORMINSTALLATIONFORM_H

#include <QObject>

QT_BEGIN_NAMESPACE
class QLabel;
class QProgressBar;
class QPushButton;
class QTimer;
class QWidget;
class QWinTaskbarButton;
class QScrollArea;
class QTextEdit;
QT_END_NAMESPACE


namespace QInstaller {

class AspectRatioLabel;
class PackageManagerCore;

class PerformInstallationForm : public QObject
{
    Q_OBJECT

public:
    explicit PerformInstallationForm(PackageManagerCore *core, QObject *parent);

    void setupUi(QWidget *widget);
    void setDetailsWidgetVisible(bool visible);
    void enableDetails();
    void startUpdateProgress();
    void stopUpdateProgress();
    void setDetailsButtonEnabled(bool enable);
    bool isShowingDetails() const;

signals:
    void showDetailsChanged();

public slots:
    void appendProgressDetails(const QString &details);
    void updateProgress();
    void toggleDetails();
    void clearDetailsBrowser();
    void onAdditionalProgressStatusChanged(const QString &status);
    void setImageFromFileName(const QString &fileName, const QString &url);

private:
    QProgressBar *m_progressBar;
    QLabel *m_progressPercentLabel;
    QLabel *m_progressLabel;
    QLabel *m_additionalProgressStatus;
    QScrollArea *m_productImagesScrollArea;
    AspectRatioLabel *m_productImagesLabel;
    QPushButton *m_detailsButton;
    QTextEdit *m_detailsBrowser;
    QTimer *m_updateTimer;
    PackageManagerCore *m_core;

#ifdef Q_OS_WIN
    QWinTaskbarButton *m_taskButton;
#endif
};

} // namespace QInstaller

#endif // PERFORMINSTALLATIONFORM_H
