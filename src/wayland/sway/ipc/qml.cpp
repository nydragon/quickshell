
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

QString SwayIpcQml::socketPath() { return SwayIpc::instance()->socketPath(); }

ObjectModel<SwayMonitor>* SwayIpcQml::monitors() { return SwayIpc::instance()->monitors(); }

ObjectModel<SwayWorkspace>* SwayIpcQml::workspaces() { return SwayIpc::instance()->workspaces(); }

} // namespace qs::sway::ipc
