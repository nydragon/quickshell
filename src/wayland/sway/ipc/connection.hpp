#pragma once

#include <qbytearrayview.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qlocalsocket.h>
#include <qobject.h>
#include <qqml.h>
#include <qqmlintegration.h>
#include <qtmetamacros.h>
#include <qtypes.h>

#include "../../../core/model.hpp"

namespace qs::sway::ipc {
class SwayWorkspace;
class SwayMonitor;
} // namespace qs::sway::ipc

Q_DECLARE_OPAQUE_POINTER(qs::sway::ipc::SwayWorkspace*);
Q_DECLARE_OPAQUE_POINTER(qs::sway::ipc::SwayMonitor*);

namespace qs::sway::ipc {

enum Message {
	GET_WORKSPACES = 1, //NOLINT
	SUBSCRIBE = 2,
	GET_OUTPUTS = 3 //NOLINT
};

class SwayIpcEvent: public QObject {
	Q_OBJECT;
	Q_PROPERTY(QString name READ nameStr CONSTANT);
	Q_PROPERTY(QString data READ dataStr CONSTANT);
	QML_NAMED_ELEMENT(SwayEvent);
	QML_UNCREATABLE("SwayIpcEvents cannot be created.");

public:
	SwayIpcEvent(QObject* parent): QObject(parent) {}

	[[nodiscard]] QString nameStr() const;
	[[nodiscard]] QString dataStr() const;

	void reset();

	QByteArrayView name;
	QJsonDocument data;
};

class SwayIpc: public QObject {
	Q_OBJECT;

public:
	static SwayIpc* instance();

	[[nodiscard]] QString socketPath() const;

	void
	makeRequest(const QByteArray& request, const std::function<void(bool, QJsonDocument)>& callback);

	static QByteArray buildRequestMessage(Message cmd, const QByteArray& payload = QByteArray());

	void refreshWorkspaces();
	void refreshMonitors();

	[[nodiscard]] ObjectModel<SwayMonitor>* monitors();
	[[nodiscard]] ObjectModel<SwayWorkspace>* workspaces();
signals:
	void connected();
	void rawEvent(SwayIpcEvent* event);

private slots:
	void eventSocketError(QLocalSocket::LocalSocketError error) const;
	void eventSocketStateChanged(QLocalSocket::LocalSocketState state);
	void eventSocketReady();

private:
	explicit SwayIpc();

	void onEvent(SwayIpcEvent* event);

	static std::tuple<QByteArrayView, QJsonDocument> parseResponse(QByteArray rawEvent);

	QLocalSocket eventSocket;
	QString mSocketPath;
	bool valid = false;

	ObjectModel<SwayMonitor> mMonitors {this};
	ObjectModel<SwayWorkspace> mWorkspaces {this};

	SwayIpcEvent event {this};
};

} // namespace qs::sway::ipc
