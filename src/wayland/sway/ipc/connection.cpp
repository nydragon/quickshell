#include "connection.hpp"

#include <qbytearrayview.h>
#include <qjsonarray.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qjsonvalue.h>
#include <qloggingcategory.h>

#include "workspace.hpp"

Q_LOGGING_CATEGORY(logSwayIpc, "quickshell.sway.ipc", QtWarningMsg);
Q_LOGGING_CATEGORY(logSwayIpcEvents, "quickshell.sway.ipc.events", QtWarningMsg);

namespace qs::sway::ipc {

void SwayIpc::makeRequest(
    const QByteArray& request,
    const std::function<void(bool, QJsonDocument)>& callback
) {

	auto* requestSocket = new QLocalSocket(this);
	qCDebug(logSwayIpc) << "Making request:" << request;

	auto connectedCallback = [this, request, requestSocket, callback]() {
		auto responseCallback = [requestSocket, callback]() {
			auto response = requestSocket->readAll();
			auto [_, data] = SwayIpc::parseResponse(response);
			callback(true, std::move(data));
			delete requestSocket;
		};

		QObject::connect(requestSocket, &QLocalSocket::readyRead, this, responseCallback);

		requestSocket->write(request);
		requestSocket->flush();
	};

	auto errorCallback = [=](QLocalSocket::LocalSocketError error) {
		qCWarning(logSwayIpc) << "Error making request:" << error << "request:" << request;
		requestSocket->deleteLater();
		callback(false, {});
	};

	QObject::connect(requestSocket, &QLocalSocket::connected, this, connectedCallback);
	QObject::connect(requestSocket, &QLocalSocket::errorOccurred, this, errorCallback);

	requestSocket->connectToServer(this->mSocketPath);
}

QByteArray SwayIpc::buildRequestMessage(Message cmd, const QByteArray& payload) {
	int len = static_cast<int>(payload.length());
	QByteArray lenBytes = QByteArray(4, Qt::Initialization::Uninitialized);
	QByteArray typeBytes = QByteArray(4, Qt::Initialization::Uninitialized);

	std::memcpy(lenBytes.data(), &len, 4);
	std::memcpy(typeBytes.data(), &cmd, 4);

	QByteArray data = QByteArray("i3-ipc") + lenBytes + typeBytes + payload;

	return data;
}

SwayIpc::SwayIpc() {
	auto sock = qEnvironmentVariable("SWAYSOCK");

	if (sock.isEmpty()) {
		qWarning() << "$SWAYSOCK is unset. Cannot connect to sway.";
		return;
	}

	this->mSocketPath = sock;
	// clang-format off
	QObject::connect(&this->eventSocket, &QLocalSocket::errorOccurred, this, &SwayIpc::eventSocketError);
	QObject::connect(&this->eventSocket, &QLocalSocket::stateChanged, this, &SwayIpc::eventSocketStateChanged);
	QObject::connect(&this->eventSocket, &QLocalSocket::readyRead, this, &SwayIpc::eventSocketReady);
	// clang-format on

	auto connectedCallback = [this]() {
		auto payload = QByteArray("[\"workspace\"]");
		auto message = SwayIpc::buildRequestMessage(Message::SUBSCRIBE, payload);

		this->eventSocket.write(message);
		this->eventSocket.flush();
	};

	QObject::connect(&this->eventSocket, &QLocalSocket::connected, this, connectedCallback);

	this->eventSocket.connectToServer(this->mSocketPath);
	this->refreshWorkspaces();
	this->refreshMonitors();
}

void SwayIpc::eventSocketReady() {
	while (true) {
		auto rawEvent = this->eventSocket.readLine();
		if (rawEvent.isEmpty()) break;

		auto [event, data] = SwayIpc::parseResponse(rawEvent);

		this->event.name = event;
		this->event.data = data;

		this->onEvent(&this->event);
		emit this->rawEvent(&this->event);
	}
}

std::tuple<QByteArrayView, QJsonDocument> SwayIpc::parseResponse(QByteArray rawEvent) {
	int headerLen = 14;

	auto byteData =
	    QByteArrayView(rawEvent.data() + headerLen, rawEvent.data() + rawEvent.length()); // NOLINT

	auto event = QByteArrayView(rawEvent.data(), headerLen);
	auto data = QJsonDocument::fromJson(byteData.toByteArray());

	return std::tuple(event, data);
}

void SwayIpc::eventSocketError(QLocalSocket::LocalSocketError error) const {
	qWarning() << error;
	if (!this->valid) {
		qWarning() << "Unable to connect to sway socket:" << error;
	} else {
		qWarning() << "Sway socket error:" << error;
	}
}

void SwayIpc::eventSocketStateChanged(QLocalSocket::LocalSocketState state) {
	qWarning() << state;
	if (state == QLocalSocket::ConnectedState) {
		qCInfo(logSwayIpc) << "Sway event socket connected.";
		emit this->connected();
	} else if (state == QLocalSocket::UnconnectedState && this->valid) {
		qCWarning(logSwayIpc) << "Hyprland event socket disconnected.";
	}

	this->valid = state == QLocalSocket::ConnectedState;
}

QString SwayIpc::socketPath() const { return this->mSocketPath; }

SwayIpc* SwayIpc::instance() {
	static SwayIpc* instance = nullptr; // NOLINT

	if (instance == nullptr) {
		instance = new SwayIpc();
	}

	return instance;
}

void SwayIpc::refreshWorkspaces() {
	auto msg = SwayIpc::buildRequestMessage(Message::GET_WORKSPACES);

	this->makeRequest(msg, [this](bool status, const QJsonDocument& data) {
		if (!status) {
			return;
		}

		auto workspaces = data.array();
		const auto& mList = this->mWorkspaces.valueList();
		auto names = QVector<QString>();

		for (auto entry: workspaces) {
			auto object = entry.toObject();
			auto name = object["name"].toString();

			auto workspaceIter = std::find_if(mList.begin(), mList.end(), [name](const SwayWorkspace* m) {
				return m->name() == name;
			});

			auto* workspace = workspaceIter == mList.end() ? nullptr : *workspaceIter;
			auto existed = workspace != nullptr;

			if (workspace == nullptr) {
				workspace = new SwayWorkspace(this);
			}

			workspace->updateFromObject(object);

			if (!existed) {
				this->mWorkspaces.insertObject(workspace);
			}

			names.push_back(name);
		}

		auto removedWorkspaces = QVector<SwayWorkspace*>();

		for (auto* workspace: mList) {
			if (!names.contains(workspace->name())) {
				removedWorkspaces.push_back(workspace);
			}
		}

		for (auto* workspace: removedWorkspaces) {
			qWarning() << workspace->name();
			this->mWorkspaces.removeObject(workspace);
			delete workspace;
		}
	});
}

void SwayIpc::refreshMonitors() {
	auto msg = SwayIpc::buildRequestMessage(Message::GET_OUTPUTS);

	this->makeRequest(msg, [](bool status, const QJsonDocument& data) {
		qWarning() << status << data; //
	});
}

void SwayIpc::onEvent(SwayIpcEvent* event) {
	if (event->data["change"] == "init") {
		qCInfo(logSwayIpcEvents) << "New workspace has been created";

		auto workspaceData = event->data["current"];

		auto* workspace = SwayIpc::findWorkspaceByName(workspaceData["name"].toString());
		bool isNew = false;

		if (workspace != nullptr) {
			workspace = new SwayWorkspace(this);
			isNew = true;
		}

		workspace->updateFromObject(workspaceData.toObject());

		if (isNew) {
			this->mWorkspaces.insertObject(workspace);
			qCInfo(logSwayIpc) << "Added workspace to list";
		}
	}

	if (event->data["change"] == "focus") {

		auto oldName = event->data["old"]["name"].toString();
		auto newName = event->data["current"]["name"].toString();

		qCInfo(logSwayIpcEvents) << "Focus changed: " << oldName << "->" << newName;

		auto* oldWorkspace = SwayIpc::findWorkspaceByName(oldName);
		auto* newWorkspace = SwayIpc::findWorkspaceByName(newName);

		if (oldWorkspace != nullptr) {
			oldWorkspace->setFocus(false);
		}

		if (newWorkspace != nullptr) {
			newWorkspace->setFocus(true);
			emit this->focusedWorkspaceChanged(newWorkspace);
		}
	}
}

SwayWorkspace* SwayIpc::findWorkspaceByName(const QString& name) {
	auto mList = this->mWorkspaces.valueList();
	auto workspaceIter = std::find_if(mList.begin(), mList.end(), [name](const SwayWorkspace* m) {
		return m->name() == name;
	});

	return workspaceIter == mList.end() ? nullptr : *workspaceIter;
}

QString SwayIpcEvent::nameStr() const { return QString::fromUtf8(this->name); }
QString SwayIpcEvent::dataStr() const { return QString::fromUtf8(this->data.toJson()); }
ObjectModel<SwayMonitor>* SwayIpc::monitors() { return &this->mMonitors; }
ObjectModel<SwayWorkspace>* SwayIpc::workspaces() { return &this->mWorkspaces; }

} // namespace qs::sway::ipc
