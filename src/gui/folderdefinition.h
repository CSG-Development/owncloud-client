#pragma once

#include "common/vfs.h"
#include <QSettings>
#include <QUuid>
#include <QUrl>

namespace CUR {

/**
 * @brief The FolderDefinition class
 * @ingroup gui
 */
class FolderDefinition
{
public:
    static auto createNewFolderDefinition(const QUrl &davUrl, const QString &spaceId, const QString &displayName = {})
    {
        return FolderDefinition(QUuid::createUuid().toByteArray(QUuid::WithoutBraces), davUrl, spaceId, displayName);
    }

    /// path to the journal, usually relative to localPath
    QString journalPath;

    /// whether the folder is paused
    bool paused = false;
    /// whether the folder syncs hidden files
    bool ignoreHiddenFiles = true;
    /// Which virtual files setting the folder uses
    Vfs::Mode virtualFilesMode = Vfs::Off;

    /// Whether the vfs mode shall silently be updated if possible
    bool upgradeVfsMode = false;

    /// Saves the folder definition into the current settings group.
    static void save(QSettings &settings, const FolderDefinition &folder);

    /// Reads a folder definition from the current settings group.
    static FolderDefinition load(QSettings &settings, const QByteArray &id);

    /// Ensure / as separator and trailing /.
    void setLocalPath(const QString &path);

    /// Remove ending /, then ensure starting '/': so "/foo/bar" and "/".
    void setTargetPath(const QString &path);

    /// journalPath relative to localPath.
    QString absoluteJournalPath() const;

    QString localPath() const;

    QString targetPath() const;

    QUrl webDavUrl() const;

    // could change in the case of spaces
    void setWebDavUrl(const QUrl &url);

    // when using spaces we don't store the dav url but the space id
    // this id is then used to look up the dav url
    QString spaceId() const;

    void setSpaceId(const QString &spaceId) { _spaceId = spaceId; }

    const QByteArray &id() const;

    QString displayName() const;

    /**
     * The folder is deployed by an admin
     * We will hide the remove option and the disable/enable vfs option.
     */
    bool isDeployed() const;

    /**
     * Higher values mean more imortant
     * Used for sorting
     */
    uint32_t priority() const;

    void setPriority(uint32_t newPriority);

private:
    FolderDefinition(const QByteArray &id, const QUrl &davUrl, const QString &spaceId, const QString &displayName);

    // oc10 and as cache for ocis
    QUrl _webDavUrl;

    QString _spaceId;
    /// For legacy reasons this can be a string, new folder objects will use a uuid
    QByteArray _id;
    QString _displayName;
    /// path on local machine (always trailing /)
    QString _localPath;
    /// path on remote (usually no trailing /, exception "/")
    QString _targetPath;
    bool _deployed = false;

    uint32_t _priority = 0;

    friend class FolderMan;
    friend class SpaceMigration;
};


} // namespace CUR
