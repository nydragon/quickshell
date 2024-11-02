
#include "workspace.hpp"

namespace qs::sway::ipc {

qint32 SwayWorkspace ::id() const { return this->mId; }
QString SwayWorkspace::name() const { return this->mName; }
bool SwayWorkspace ::urgent() const { return this->mUrgent; }
bool SwayWorkspace::focused() const { return this->mFocused; }

void SwayWorkspace::updateFromObject(QJsonObject obj) {

	if (obj["id"].isDouble()) {
		this->mId = obj["id"].toInt(-1);
		emit this->idChanged();
	}

	if (obj["name"].isString()) {
		this->mName = obj["name"].toString();
		emit this->nameChanged();
	}

	if (obj["urgent"].isBool()) {
		this->mUrgent = obj["urgent"].toBool();
	}

	if (obj["focused"].isString()) {
		this->mFocused = obj["focused"].toBool();
	}
}

void SwayWorkspace::setFocus(bool data) { this->mFocused = data; }

} // namespace qs::sway::ipc
