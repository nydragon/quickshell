
#include "qml.hpp"

#include "connection.hpp"

namespace qs::sway::ipc {

SwayIpcQml::SwayIpcQml() {
	auto* instance = SwayIpc::instance();

	QObject::connect(instance, &SwayIpc::rawEvent, this, &SwayIpcQml::rawEvent);
	QObject::connect(instance, &SwayIpc::connected, this, &SwayIpcQml::connected);
	QObject::connect(
	    instance,
	    &SwayIpc::focusedWorkspaceChanged,
	    this,
	    &SwayIpcQml::focusedWorkspaceChanged
	);
}

void SwayIpcQml::dispatch(const QString& request) { SwayIpc::instance()->dispatch(request); }

void SwayIpcQml::refreshMonitors() { SwayIpc::instance()->refreshMonitors(); }

void SwayIpcQml::refreshWorkspaces() { SwayIpc::instance()->refreshWorkspaces(); }

QString SwayIpcQml::socketPath() { return SwayIpc::instance()->socketPath(); }

ObjectModel<SwayMonitor>* SwayIpcQml::monitors() { return SwayIpc::instance()->monitors(); }

ObjectModel<SwayWorkspace>* SwayIpcQml::workspaces() { return SwayIpc::instance()->workspaces(); }

} // namespace qs::sway::ipc
