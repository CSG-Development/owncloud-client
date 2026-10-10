/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "readyforinstallationpage_p.h"

#include "ui_componentsummary.h"
#include "settings.h"
#include "componentselectedproxymodel.h"
#include "componentmodel.h"
#include "globals.h"
#include "fileutils.h"
#include "spacewidget.h"

#include <QLabel>
#include <QLayout>
#include <QTextBrowser>
#include <QTreeView>
#include <QHeaderView>

const uint scIndentation = 15;
const uint scTitleSizeIncrease = 3;

namespace QInstaller {

TreeItemDelegate::TreeItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void TreeItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    QStyledItemDelegate::initStyleOption(&opt, index);
    if (!index.parent().isValid() && index.column() == ComponentModelHelper::NameColumn) {
        QFont font = opt.font;
        font.setBold(true);
        opt.font = font;
    } else if (index.column() == ComponentModelHelper::NameColumn) {
        int depth = 0;
        QModelIndex tmpIndex = index;
        forever {
            if (!tmpIndex.parent().isValid())
                break;
            depth++;
            tmpIndex = tmpIndex.parent();
        }
        opt.rect.adjust(scIndentation * depth, 0, 0, 0);
    }
    opt.features &= ~QStyleOptionViewItem::HasCheckIndicator;

    QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
    if (index.column() == ComponentModelHelper::UncompressedSizeColumn) {
        style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

        // Column has for some reason saved space for checkbox indicator, although we have remove it with
        // ~QStyleOptionViewItem::HasCheckIndicator. Drawing the text here to avoid gap between text and
        // column edge. Without this fix the text has sometimes ellipsis although there would be enough space.
        QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &opt, opt.widget);
        int checkboxWidth = style->pixelMetric(QStyle::PM_IndicatorWidth, &opt);
        textRect.adjust(-checkboxWidth, 0, 0, 0); // shift text left

        painter->save();
        painter->setFont(opt.font);
        painter->setPen(opt.palette.color(QPalette::Text));
        painter->drawText(textRect, Qt::AlignRight | Qt::AlignVCenter, index.data().toString());
        painter->restore();
    } else {
        style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);
    }
}

QSize TreeItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const int kRowExtraHeight = 3;
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.setHeight(size.height() + kRowExtraHeight);
    return size;
}

ReadyForInstallationPagePrivate::ReadyForInstallationPagePrivate(ReadyForInstallationPage *qq, PackageManagerCore *core)
    : m_ui(new Ui::ComponentSummaryForm)
    , q(qq)
    , m_core(core)
    , m_spaceWidget(nullptr)
    , m_componentsSelectedProxyModel(nullptr)
    , m_componentsUnselectedProxyModel(nullptr)
{
    m_ui->setupUi(q);
    m_ui->UninstallComponentsWidget->setVisible(false);
    m_ui->InstallComponentsWidget->setVisible(false);

    m_ui->ComponentSummaryScrollArea->setStyleSheet(QLatin1String(
        "#ComponentSummaryScrollArea, #ComponentSummaryScrollArea > QWidget,"
        " #scrollAreaWidgetContents{background:#ffffff;}"));

    initializeTitleLabel(m_ui->InstallMsgLabel);
    initializeTitleLabel(m_ui->UninstallMsgLabel);

    if (m_core->settings().wizardShowPageList())
        m_ui->mainVLayout->setContentsMargins(QMargins(0, -1, -1, -1));
}

ReadyForInstallationPagePrivate::~ReadyForInstallationPagePrivate()
{
    delete m_ui;
}

bool ReadyForInstallationPagePrivate::entering()
{
    bool isComplete = false;
    m_ui->RemoveAllMsgLabel->hide();
    m_ui->UninstallComponentsWidget->setVisible(false);
    m_ui->InstallComponentsWidget->setVisible(false);

    if (m_core->isUpdater()) {
        initializeInstallTree(tr("You are updating"));
        q->setButtonText(QWizard::CommitButton, tr("U&pdate"));
    } else if (m_core->isPackageManager()) {
        initializeInstallTree(tr("You are installing"));
        initializeUninstallTree();
        q->setButtonText(QWizard::CommitButton, tr("U&pdate"));
    } else if (m_core->isUninstaller()) {
        initializeUninstaller();
        q->setButtonText(QWizard::CommitButton, tr("U&ninstall"));
        showSpaceWidget(false);
        return true;
    } else if (m_core->isOfflineGenerator()) {
        initializeInstallTree(tr("You are including following packages"));
        q->setButtonText(QWizard::CommitButton, tr("Create Offline Installer"));
    } else {
        Q_ASSERT(m_core->isInstaller());
        initializeInstallTree(tr("You are installing"));
        q->setButtonText(QWizard::CommitButton, tr("&Install"));
    }

    isComplete = m_core->recalculateAllComponents();
    const QString htmlOutput = m_core->componentResolveReasons();
    qCDebug(QInstaller::lcInstallerInstallLog).noquote() << htmlToString(htmlOutput);

    showSpaceWidget(true);

    if (!m_core->checkAvailableSpace())
        isComplete = false;

    return isComplete;
}

void ReadyForInstallationPagePrivate::initializeTitleLabel(QLabel *label)
{
    QFont labelFont = label->font();
    QFontInfo info = label->fontInfo();
    labelFont.setPixelSize(info.pixelSize() + scTitleSizeIncrease);
    label->setFont(labelFont);
}

void ReadyForInstallationPagePrivate::initializeInstallTree(const QString &title)
{
    m_ui->InstallMsgLabel->setText(title);
    if (m_componentsSelectedProxyModel) {
        m_componentsSelectedProxyModel->setSourceModel(m_core->isUpdater() ? m_core->updaterComponentModel() : m_core->defaultComponentModel());
        updateView(m_componentsSelectedProxyModel,  m_ui->InstallComponentsTreeview, m_ui->InstallComponentsWidget);
        return;
    }
    m_componentsSelectedProxyModel = new ComponentSelectedProxyModel(m_core, Selected, this);
    initializeTree(m_componentsSelectedProxyModel, m_ui->InstallComponentsTreeview, m_ui->InstallComponentsWidget);
}

void ReadyForInstallationPagePrivate::initializeUninstallTree()
{
    if (m_componentsUnselectedProxyModel) {
        updateView(m_componentsUnselectedProxyModel, m_ui->UninstallComponentsTreeview, m_ui->UninstallComponentsWidget);
        return;
    }
    m_componentsUnselectedProxyModel = new ComponentSelectedProxyModel(m_core, Unselected, this);
    initializeTree(m_componentsUnselectedProxyModel, m_ui->UninstallComponentsTreeview, m_ui->UninstallComponentsWidget);
}

void ReadyForInstallationPagePrivate::initializeTree(QSortFilterProxyModel *model, QTreeView *treeView, QWidget *container)
{
    model->setSourceModel(m_core->isUpdater() ? m_core->updaterComponentModel() : m_core->defaultComponentModel());
    treeView->setModel(model);

    if (model->rowCount() > 0)
        container->setVisible(true);

    decorateTreeView(treeView);
    adjustTreeWidgetHeight(treeView);
}

void ReadyForInstallationPagePrivate::initializeUninstaller()
{
    m_ui->RemoveAllMsgLabel->show();
    m_ui->RemoveAllMsgLabel->setText(tr("All required information is now available to begin removing %1 from your computer.<br>"
                                "<font color=\"red\">The program directory %2 will be deleted completely</font>, "
                                "including all content in that directory!").arg(q->productName(),
                                QDir::toNativeSeparators(QDir(m_core->value(scTargetDir)).absolutePath())));
}

void ReadyForInstallationPagePrivate::updateView(QSortFilterProxyModel *model, QTreeView *treeView, QWidget *container)
{
    model->invalidate();
    if (model->rowCount() == 0) {
        container->setVisible(false);
    } else {
        container->setVisible(true);
        treeView->expandAll();
        adjustTreeWidgetHeight(treeView);
    }
}

void ReadyForInstallationPagePrivate::decorateTreeView(QTreeView *treeView)
{
    treeView->setHeaderHidden(true);
    treeView->header()->setSectionResizeMode(ComponentModelHelper::NameColumn, QHeaderView::Stretch);
    treeView->header()->setStretchLastSection(false);
    treeView->hideColumn(ComponentModelHelper::ActionColumn);
    treeView->hideColumn(ComponentModelHelper::InstalledVersionColumn);
    treeView->hideColumn(ComponentModelHelper::NewVersionColumn);
    treeView->hideColumn(ComponentModelHelper::ReleaseDateColumn);

    treeView->setIndentation(0);
    treeView->setRootIsDecorated(false);
    treeView->expandAll();
    treeView->setItemDelegate(new TreeItemDelegate(this));
}

void ReadyForInstallationPagePrivate::adjustTreeWidgetHeight(QTreeView *treeView)
{
    int totalHeight = treeView->header()->height();
    QAbstractItemModel *model = treeView->model();

    std::function<void(const QModelIndex&)> calculateHeight = [&](const QModelIndex& index) {
        totalHeight += treeView->visualRect(index).height();
        for (int i = 0; i < model->rowCount(index); ++i)
            calculateHeight(model->index(i, 0, index));
    };

    for (int row = 0; row < model->rowCount(); ++row)
        calculateHeight(model->index(row, 0));

    treeView->setFixedHeight(totalHeight);
}

void ReadyForInstallationPagePrivate::showSpaceWidget(bool show)
{
    if (!show) {
        if (m_spaceWidget)
            m_spaceWidget->hide();
        return;
    }

    if (!m_spaceWidget) {
        m_spaceWidget = new SpaceWidget(m_core, true);
        m_ui->mainVLayout->addWidget(m_spaceWidget);
    } else {
        m_spaceWidget->show();
        m_spaceWidget->updateSpaceRequiredText();
    }
}

}  // namespace QInstaller
