qt_add_library(quickshell-sway STATIC)

target_link_libraries(quickshell-sway PRIVATE ${QT_DEPS})

set(SWAY_MODULES)

if (SWAY_IPC)
	add_subdirectory(ipc)
	list(APPEND SWAY_MODULES Quickshell.Sway._Ipc)
endif()

qt_add_qml_module(quickshell-sway
	URI Quickshell.Sway
	VERSION 0.1
	IMPORTS ${SWAY_MODULES}
)

qs_pch(quickshell-sway)
qs_pch(quickshell-swayplugin)

target_link_libraries(quickshell PRIVATE quickshell-swayplugin)
