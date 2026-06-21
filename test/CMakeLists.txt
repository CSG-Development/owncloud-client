include(personalcloud_add_test.cmake)

add_subdirectory(testutils)

personalcloud_add_test(JHash)

personalcloud_add_test(PersonalCloudPropagator)
personalcloud_add_test(OwnSql)
personalcloud_add_test(SyncJournalDB)
personalcloud_add_test(SyncFileItem)
personalcloud_add_test(ConcatUrl)
personalcloud_add_test(XmlParse)
personalcloud_add_test(ChecksumValidator)


# TODO: we need keychain access for this test
if (NOT APPLE OR NOT DEFINED ENV{GITHUB_ACTION})
    personalcloud_add_test(CredentialManager)
endif()

personalcloud_add_test(ExcludedFiles)

personalcloud_add_test(Utility)

personalcloud_add_test(SyncEngine)
personalcloud_add_test(SyncVirtualFiles)
personalcloud_add_test(SyncMove)
add_dependencies(SyncMoveTest test_helper)
personalcloud_add_test(SyncDelete)
personalcloud_add_test(SyncConflict)
personalcloud_add_test(SyncFileStatusTracker)
personalcloud_add_test(Download)
personalcloud_add_test(ChunkingNg)
personalcloud_add_test(UploadReset)
personalcloud_add_test(AllFilesDeleted)
personalcloud_add_test(Blacklist)
personalcloud_add_test(LocalDiscovery)
personalcloud_add_test(RemoteDiscovery)
personalcloud_add_test(Permissions)
personalcloud_add_test(SelectiveSync)
personalcloud_add_test(DatabaseError)
personalcloud_add_test(LockedFiles)

if (WITH_AUTO_UPDATER)
    personalcloud_add_test(Updater)
endif()

personalcloud_add_test(FolderMigration)

personalcloud_add_test(FolderWatcher)

personalcloud_add_test(LongPath)

personalcloud_add_test(FolderMan)

personalcloud_add_test(OAuth)

configure_file(test_journal.db "${PROJECT_BINARY_DIR}/bin/test_journal.db" COPYONLY)


personalcloud_add_test(JobQueue)
personalcloud_add_test(SpacesMigration)

add_subdirectory(modeltests)
