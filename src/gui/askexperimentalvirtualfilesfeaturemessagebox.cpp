#include "askexperimentalvirtualfilesfeaturemessagebox.h"
#include "customdialogs/custommessagebox.h"

#include <QWidget>

namespace APP {

CustomMessageBox* CreateExperimentalVirtualFilesFeatureMessageBox(QWidget *parent) {
    auto messageBox = new CustomMessageBox(parent);
    messageBox->setHeaderText(QObject::tr("Enable experimental feature?"))
        .setMessageText(QObject::tr("When the \"virtual files\" mode is enabled no files will be downloaded initially. "
                           "Instead, a tiny file will be created for each file that exists on the server. "
                           "The contents can be downloaded by running these files or by using their context menu."
                           "\n\n"
                           "The virtual files mode is mutually exclusive with selective sync. "
                           "Currently unselected folders will be translated to online-only folders "
                           "and your selective sync settings will be reset."
                           "\n\n"
                           "Switching to this mode will abort any currently running synchronization."
                           "\n\n"
                           "This is a new, experimental mode. If you decide to use it, please report any "
                           "issues that come up."))
        .setAcceptButtonText(QObject::tr("Enable feature"))
        .setRejectButtonText(QObject::tr("Stay safe"))
        .setWide(true)
        .setWarningIconVisible(true)
        .setDeleteOnClose(true);
    return messageBox;
}

}
