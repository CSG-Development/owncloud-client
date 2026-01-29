#include "codeinputwidget.h"
#include "ui_codeinputwidget.h"
#include "stylehelper.h"
#include "theme.h"

#include <QKeyEvent>
#include <QClipboard>
#include <QMimeData>
#include <QMenu>
#include <QRegularExpressionValidator>

namespace {
const QPair<QString,QString> widgetStyle = {
    QStringLiteral(":/res/inputwidget/codeinputwidget_light.qss"),
    QStringLiteral(":/res/inputwidget/codeinputwidget_dark.qss")
};
const QPair<QString,QString> widgetStyleError = {
    QStringLiteral(":/res/inputwidget/codeinputwidget_error_light.qss"),
    QStringLiteral(":/res/inputwidget/codeinputwidget_error_dark.qss")
};
constexpr auto ed_count = 6;
}

CodeInputWidget::CodeInputWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CodeInputWidget)
{
    ui->setupUi(this);

    edPtrs.append(ui->ed1);
    edPtrs.append(ui->ed2);
    edPtrs.append(ui->ed3);
    edPtrs.append(ui->ed4);
    edPtrs.append(ui->ed5);
    edPtrs.append(ui->ed6);

    pasteCodeAction = new QAction(tr("Paste code"), this);
    connect(pasteCodeAction, &QAction::triggered, this, &CodeInputWidget::pasteCode);

    auto validator = new QRegularExpressionValidator(QRegularExpression(QStringLiteral("[\\d]")), this);

    for (int i = 0; i < ed_count; i++) {
        connect(edPtrs[i], &QLineEdit::textEdited, this, &CodeInputWidget::onTextEdited);
        edPtrs[i]->installEventFilter(this);
        edPtrs[i]->setValidator(validator);

        edPtrs[i]->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(edPtrs[i], &QLineEdit::customContextMenuRequested, this, &CodeInputWidget::onContextMenuRequested);
    }

    ui->ed1->setFocus();

    themeNotifier = darkTheme_.addNotifier([this] {
        updateStyles();
    });
    updateStyles();
}

CodeInputWidget::~CodeInputWidget()
{
    delete ui;
}

QString CodeInputWidget::codeStr() const
{
    QString s = code_;

    if (s.length() != ed_count)
        return {};

    return s;
}

void CodeInputWidget::clearCode()
{
    for (int i = 0; i < ed_count; i++) {
        edPtrs[i]->clear();
    }
    code_.clear();
}

void CodeInputWidget::setErrorState(bool enable)
{
    errorState = enable;
    updateStyles();
}

bool CodeInputWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        if (auto key_ev = static_cast<QKeyEvent*>(event)) {
            if (key_ev->key() == Qt::Key_Backspace) {
                if (auto ed = static_cast<QLineEdit*>(obj)) {
                    if (ed->text().isEmpty()) {
                        moveFocus(false);
                        return true;
                    }
                }
            }
            else if (
                (key_ev->key() == Qt::Key_V && key_ev->modifiers() & Qt::ControlModifier) ||
                (key_ev->key() == Qt::Key_Insert && key_ev->modifiers() & Qt::ShiftModifier)) {
                pasteCode();
                return true;
            }
        }
    }

    // Easy overwrite code digits if the field is not empty
    if (event->type() == QEvent::FocusIn) {
        if (auto ed = static_cast<QLineEdit*>(obj)) {
            if (!ed->text().isEmpty()) {
                ed->selectAll();
                if (errorState) {
                    setErrorState(false);
                    emit focusGained();
                }
            }
        }
    }

    return QWidget::eventFilter(obj, event);
}

void CodeInputWidget::updateStyles()
{
    if (errorState)
        setStyleSheet(CUR::StyleHelper::loadFileToString(darkTheme_.value() ? widgetStyleError.second : widgetStyleError.first));
    else
        setStyleSheet(CUR::StyleHelper::loadFileToString(darkTheme_.value() ? widgetStyle.second : widgetStyle.first));

    style()->unpolish(this);
    style()->polish(this);
    update();
}

void CodeInputWidget::onTextEdited(const QString &/*txt*/)
{
    auto ed = qobject_cast<QLineEdit*>(sender());
    if (!ed)
        return;

    if (!ed->text().isEmpty())
        moveFocus(true);

    code_ = buildCode();
    setErrorState(false);
    emit codeChanged();
}

void CodeInputWidget::moveFocus(bool forward)
{
    int idx = indexOfEdit(focusWidget());
    if (idx == -1)
        return;
    int new_idx = forward ? idx + 1 : idx - 1;
    new_idx = qBound(0, new_idx, ed_count - 1);
    edPtrs[new_idx]->setFocus();
}

int CodeInputWidget::indexOfEdit(QWidget* ed)
{
    return edPtrs.indexOf(ed);
}

bool CodeInputWidget::validateClipboardBuffer()
{
    const QClipboard* clipboard = QApplication::clipboard();
    const QMimeData* mimeData = clipboard->mimeData();
    return (mimeData->hasText() && mimeData->text().length() == 6);
}

void CodeInputWidget::pasteCode()
{
    if (!validateClipboardBuffer()) {
        return;
    }
    const QClipboard* clipboard = QApplication::clipboard();
    const QMimeData* mimeData = clipboard->mimeData();
    const auto txt = mimeData->text();

    for (int i = 0; i < ed_count; i++) {
        const auto b_ = QSignalBlocker(edPtrs[i]);
        QString tmp;
        tmp.append(txt[i]);
        edPtrs[i]->setText(tmp);
    }

    edPtrs[ed_count - 1]->setFocus();

    code_ = buildCode();
    emit codeChanged();
}

void CodeInputWidget::onContextMenuRequested(const QPoint &/*pos*/)
{
    QMenu m;
    m.addAction(pasteCodeAction);
    pasteCodeAction->setEnabled(validateClipboardBuffer());
    m.exec(QCursor::pos());
}

QString CodeInputWidget::buildCode() const
{
    QString code;
    code += ui->ed1->text();
    code += ui->ed2->text();
    code += ui->ed3->text();
    code += ui->ed4->text();
    code += ui->ed5->text();
    code += ui->ed6->text();
    return code;
}
