#pragma once

#include <QDialog>
#include <QPoint>
#include <QtGlobal>

namespace Ui
{
class OnboardingDlg;
}

class QCloseEvent;
class QEvent;
class QObject;

class OnboardingDlg : public QDialog
{
    Q_OBJECT

public:
    explicit OnboardingDlg(QWidget *parent = nullptr);
    ~OnboardingDlg();

    int currentPage() const;
    void setCurrentPage(int page);

signals:
    void pageShown(int page);
    void completed();
    void dismissed();

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void showPreviousPage();
    void showNextPage();
    void skip();
    void updateTheme(bool isDark);
    void updateNavigation();

private:
    Ui::OnboardingDlg *ui = nullptr;
    bool _completed = false;
    bool _dismissed = false;
#ifdef Q_OS_WINDOWS
    bool _dragging = false;
    QPoint _dragStartGlobalPosition;
    QPoint _dragStartWindowPosition;
#endif
};
