#pragma once

#include "connection.hpp"

namespace qs::sway::ipc {

class SwayWorkspace: public QObject {
	Q_OBJECT;

	Q_PROPERTY(qint32 id READ id NOTIFY idChanged);
	Q_PROPERTY(QString name READ name NOTIFY nameChanged);
	Q_PROPERTY(bool urgent READ urgent);
	Q_PROPERTY(bool focused READ focused);

	QML_ELEMENT;
	QML_UNCREATABLE("SwayWorkspaces must be retrieved from the Sway object.");

public:
	SwayWorkspace(SwayIpc* ipc): QObject(ipc), ipc(ipc) {}

	[[nodiscard]] qint32 id() const;
	[[nodiscard]] QString name() const;
	[[nodiscard]] bool urgent() const;
	[[nodiscard]] bool focused() const;

	void updateFromObject(QJsonObject obj);
	void setFocus(bool);

signals:
	void idChanged();
	void nameChanged();

private:
	SwayIpc* ipc;

	qint32 mId = -1;
	QString mName;
	bool mFocused = false;
	bool mUrgent = false;
	QVariantMap mLastIpcObject;
	SwayMonitor* mMonitor = nullptr;
};

} // namespace qs::sway::ipc
