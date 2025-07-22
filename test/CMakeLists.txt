include(curator_add_test.cmake)

add_subdirectory(testutils)

curator_add_test(JHash)

curator_add_test(CuratorPropagator)
curator_add_test(OwnSql)
curator_add_test(SyncJournalDB)
curator_add_test(SyncFileItem)
curator_add_test(ConcatUrl)
curator_add_test(XmlParse)
curator_add_test(ChecksumValidator)


# TODO: we need keychain access for this test
if (NOT APPLE OR NOT DEFINED ENV{GITHUB_ACTION})
    curator_add_test(CredentialManager)
endif()

curator_add_test(ExcludedFiles)

curator_add_test(Utility)

curator_add_test(SyncEngine)
curator_add_test(SyncVirtualFiles)
curator_add_test(SyncMove)
add_dependencies(SyncMoveTest test_helper)
curator_add_test(SyncDelete)
curator_add_test(SyncConflict)
curator_add_test(SyncFileStatusTracker)
curator_add_test(Download)
curator_add_test(ChunkingNg)
curator_add_test(UploadReset)
curator_add_test(AllFilesDeleted)
curator_add_test(Blacklist)
curator_add_test(LocalDiscovery)
curator_add_test(RemoteDiscovery)
curator_add_test(Permissions)
curator_add_test(SelectiveSync)
curator_add_test(DatabaseError)
curator_add_test(LockedFiles)

if (WITH_AUTO_UPDATER)
    curator_add_test(Updater)
endif()

curator_add_test(FolderMigration)

curator_add_test(FolderWatcher)

if( UNIX AND NOT APPLE )
    curator_add_test(InotifyWatcher)
endif(UNIX AND NOT APPLE)


curator_add_test(LongPath)

curator_add_test(FolderMan)

curator_add_test(OAuth)

configure_file(test_journal.db "${PROJECT_BINARY_DIR}/bin/test_journal.db" COPYONLY)


curator_add_test(JobQueue)
curator_add_test(SpacesMigration)

add_subdirectory(modeltests)
