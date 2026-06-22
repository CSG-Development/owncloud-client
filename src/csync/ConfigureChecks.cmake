include(CheckFunctionExists)

set(SYSCONFDIR ${CMAKE_INSTALL_SYSCONFDIR})

check_function_exists(utimes HAVE_UTIMES)
check_function_exists(lstat HAVE_LSTAT)
