#pragma once

#include <qobject.h>

#include "../../../core/doc.hpp"
#include "connection.hpp"

namespace qs::sway::ipc {

class SwayIpcQml: public QObject {
	Q_OBJECT;
	// clang-format off
	/// Path to the Sway socket
	Q_PROPERTY(QString socketPath READ socketPath CONSTANT);
	/// All sway monitors.
	QSDOC_TYPE_OVERRIDE(ObjectModel<qs::sway::ipc::SwayMonitor>*);
	Q_PROPERTY(UntypedObjectModel* monitors READ monitors CONSTANT);
	/// All sway workspaces.
	QSDOC_TYPE_OVERRIDE(ObjectModel<qs::sway::ipc::SwayWorkspace>*);
	Q_PROPERTY(UntypedObjectModel* workspaces READ workspaces CONSTANT);
	// clang-format on
	QML_NAMED_ELEMENT(Sway);
	QML_SINGLETON;

public:
	explicit SwayIpcQml();

	[[nodiscard]] static QString socketPath();
	[[nodiscard]] static ObjectModel<SwayMonitor>* monitors();
	[[nodiscard]] static ObjectModel<SwayWorkspace>* workspaces();
signals:
	void rawEvent(SwayIpcEvent* event);
	void connected();
	void focusedWorkspaceChanged(SwayWorkspace* workspace);
};

} // namespace qs::sway::ipc
