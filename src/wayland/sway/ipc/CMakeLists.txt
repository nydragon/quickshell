qt_add_library(quickshell-sway-ipc STATIC
	connection.cpp
	qml.cpp
	workspace.cpp
	monitor.cpp
)

qt_add_qml_module(quickshell-sway-ipc
    URI Quickshell.Sway._Ipc
    VERSION 0.1
    DEPENDENCIES QtQml Quickshell
)

target_link_libraries(quickshell-sway-ipc PRIVATE ${QT_DEPS})

qs_pch(quickshell-sway-ipc)
qs_pch(quickshell-sway-ipcplugin)

target_link_libraries(quickshell PRIVATE quickshell-sway-ipcplugin)
