configure_file(AppRun.in AppRun @ONLY)
configure_file(qttube.desktop.in qttube.desktop @ONLY)
configure_file(win_install.nsi.in win_install.nsi @ONLY)

install(FILES portable.txt DESTINATION "." COMPONENT portable EXCLUDE_FROM_ALL)
install(FILES selfcontained.txt DESTINATION "." COMPONENT selfcontained EXCLUDE_FROM_ALL)
