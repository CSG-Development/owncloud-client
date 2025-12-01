#include "folderdefinition.h"

namespace CUR {


bool FolderDefinition::isDeployed() const
{
    return _deployed;
}

QUrl FolderDefinition::webDavUrl() const
{
    Q_ASSERT(_webDavUrl.isValid());
    return _webDavUrl;
}

void FolderDefinition::setWebDavUrl(const QUrl &url)
{
    _webDavUrl = url;
}

QString FolderDefinition::targetPath() const
{
    return _targetPath;
}

QString FolderDefinition::localPath() const
{
    return _localPath;
}

QString FolderDefinition::spaceId() const
{
    // we might call the function to check for the id
    // anyhow one of the conditions needs to be true
    Q_ASSERT(_webDavUrl.isValid() || !_spaceId.isEmpty());
    return _spaceId;
}

} // namespace CUR
