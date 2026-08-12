for /f "delims=" %%d in ( 'vswhere.exe -latest -property installationPath' ) do @( call "%%d\VC\Auxiliary\Build\vcvars64.bat" )
chdir /d %GITHUB_WORKSPACE%\install\bin
windeployqt -sql -svg --exclude-plugins qsqlibase,qsqlmimer,qsqloci,qsqlodbc,qsqlpsql --no-compiler-runtime --no-system-d3d-compiler --release qttube.exe
copy "%QT_ROOT_DIR%\resources\v8_context_snapshot.bin" resources\v8_context_snapshot.bin
