
cd win64

@REM: Release
windeployqt.exe --dir qtdata --libdir . --plugindir qtdata/plugins --no-system-d3d-compiler --no-opengl-sw --release  --qmldir res/qml Zephyr.exe

@REM Debug:
windeployqt.exe --dir qtdata --libdir . --plugindir qtdata/plugins --no-system-d3d-compiler --compiler-runtime --no-opengl-sw --debug  --qmldir res/qml Zephyr.exe
