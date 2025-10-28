#include "codeinputwidget.h"
#include "ui_codeinputwidget.h"

#include <QKeyEvent>
#include <QClipboard>
#include <QMimeData>
#include <QMenu>
#include <QRegularExpressionValidator>

namespace {
const auto widgetStyle = QStringLiteral("#ed1, #ed2, #ed3, #ed4, #ed5, #ed6 {"
                             "border-radius: 20px;"
                             "border: 1px solid #CBCDD3;"
                             "font-size: 16px;"
                             "}");
constexpr auto ed_count = 6;
}

CodeInputWidget::CodeInputWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CodeInputWidget)
{
    ui->setupUi(this);

    ui->frame->setStyleSheet(widgetStyle);

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
        connect(edPtrs[i], &QLineEdit::textChanged, this, &CodeInputWidget::onTextEdited);
        edPtrs[i]->installEventFilter(this);
        edPtrs[i]->setValidator(validator);

        edPtrs[i]->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(edPtrs[i], &QLineEdit::customContextMenuRequested, this, &CodeInputWidget::onContextMenuRequested);
    }

    edPtrs[0]->setFocus();
}

CodeInputWidget::~CodeInputWidget()
{
    delete ui;
}

QString CodeInputWidget::codeStr() const
{
    QString s;
    for (int i = 0; i < ed_count; i++) {
        s += edPtrs[i]->text().trimmed();
    }
    if (s.length() != ed_count)
        return {};
    return s;
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
        }
    }

    return QWidget::eventFilter(obj, event);
}

void CodeInputWidget::onTextEdited(const QString &/*txt*/)
{
    auto ed = qobject_cast<QLineEdit*>(sender());
    if (!ed)
        return;

    if (!ed->text().isEmpty())
        moveFocus(true);

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
        edPtrs[i]->setText(txt[i]);
    }
    edPtrs[ed_count - 1]->setFocus();
    emit codeChanged();
}

void CodeInputWidget::onContextMenuRequested(const QPoint &/*pos*/)
{
    QMenu m;
    m.addAction(pasteCodeAction);
    pasteCodeAction->setEnabled(validateClipboardBuffer());
    m.exec(QCursor::pos());
}
