#include "core.hpp"

#include <qcontainerfwd.h>
#include <qdbusconnection.h>
#include <qdbusconnectioninterface.h>
#include <qdbusextratypes.h>
#include <qdbuspendingcall.h>
#include <qdbuspendingreply.h>
#include <qdbusservicewatcher.h>
#include <qlogging.h>
#include <qloggingcategory.h>
#include <qobject.h>
#include <qqmllist.h>
#include <qtmetamacros.h>

#include "../../core/model.hpp"
#include "../../dbus/bus.hpp"
#include "../../dbus/properties.hpp"
#include "dbus_service.h"
#include "device.hpp"

namespace qs::service::upower {

Q_LOGGING_CATEGORY(logUPower, "quickshell.service.upower", QtWarningMsg);

UPower::UPower() {
	qCDebug(logUPower) << "Starting UPower Service";

	auto bus = QDBusConnection::systemBus();

	if (!bus.isConnected()) {
		qCWarning(logUPower) << "Could not connect to DBus. UPower service will not work.";
		return;
	}

	this->service =
	    new DBusUPowerService("org.freedesktop.UPower", "/org/freedesktop/UPower", bus, this);

	if (!this->service->isValid()) {
		qCDebug(logUPower) << "UPower service is not currently running, attempting to start it.";

		dbus::tryLaunchService(this, bus, "org.freedesktop.UPower", [this](bool success) {
			if (success) {
				qCDebug(logUPower) << "Successfully launched UPower service.";
				this->init();
			} else {
				qCWarning(logUPower) << "Could not start UPower. The UPower service will not work.";
			}
		});
	} else {
		this->init();
	}
}

void UPower::init() {
	this->serviceProperties.setInterface(this->service);
	this->serviceProperties.updateAllViaGetAll();

	this->registerExisting();
}

void UPower::registerExisting() {
	this->registerDevice("/org/freedesktop/UPower/devices/DisplayDevice");

	auto pending = this->service->EnumerateDevices();
	auto* call = new QDBusPendingCallWatcher(pending, this);

	auto responseCallback = [this](QDBusPendingCallWatcher* call) {
		const QDBusPendingReply<QList<QDBusObjectPath>> reply = *call;

		if (reply.isError()) {
			qCWarning(logUPower) << "Failed to enumerate devices:" << reply.error().message();
		} else {
			for (const QDBusObjectPath& devicePath: reply.value()) {
				this->registerDevice(devicePath.path());
			}
		}

		delete call;
	};

	QObject::connect(call, &QDBusPendingCallWatcher::finished, this, responseCallback);
}

void UPower::onDeviceReady() {
	auto* device = qobject_cast<UPowerDevice*>(this->sender());

	if (device->path() == "/org/freedesktop/UPower/devices/DisplayDevice") {
		this->mDisplayDevice = device;
		emit this->displayDeviceChanged();
		qCDebug(logUPower) << "Display UPowerDevice" << device->path() << "ready";
		return;
	}

	this->readyDevices.insertObject(device);
	qCDebug(logUPower) << "UPowerDevice" << device->path() << "ready";
}

void UPower::onDeviceDestroyed(QObject* object) {
	auto* device = static_cast<UPowerDevice*>(object); // NOLINT

	this->mDevices.remove(device->path());

	if (device == this->mDisplayDevice) {
		this->mDisplayDevice = nullptr;
		emit this->displayDeviceChanged();
		qCDebug(logUPower) << "Display UPowerDevice" << device->path() << "destroyed";
		return;
	}

	this->readyDevices.removeObject(device);
	qCDebug(logUPower) << "UPowerDevice" << device->path() << "destroyed";
}

void UPower::registerDevice(const QString& path) {
	if (this->mDevices.contains(path)) {
		qCDebug(logUPower) << "Skipping duplicate registration of UPowerDevice" << path;
		return;
	}

	auto* device = new UPowerDevice(path, this);
	if (!device->isValid()) {
		qCWarning(logUPower) << "Ignoring invalid UPowerDevice registration of" << path;
		delete device;
		return;
	}

	this->mDevices.insert(path, device);
	QObject::connect(device, &UPowerDevice::ready, this, &UPower::onDeviceReady);
	QObject::connect(device, &QObject::destroyed, this, &UPower::onDeviceDestroyed);

	qCDebug(logUPower) << "Registered UPowerDevice" << path;
}

UPowerDevice* UPower::displayDevice() { return this->mDisplayDevice; }

ObjectModel<UPowerDevice>* UPower::devices() { return &this->readyDevices; }

UPower* UPower::instance() {
	static UPower* instance = new UPower(); // NOLINT
	return instance;
}

UPowerQml::UPowerQml(QObject* parent): QObject(parent) {
	QObject::connect(
	    UPower::instance(),
	    &UPower::displayDeviceChanged,
	    this,
	    &UPowerQml::displayDeviceChanged
	);
	QObject::connect(
	    UPower::instance(),
	    &UPower::onBatteryChanged,
	    this,
	    &UPowerQml::onBatteryChanged
	);
}

UPowerDevice* UPowerQml::displayDevice() { // NOLINT
	return UPower::instance()->displayDevice();
}

ObjectModel<UPowerDevice>* UPowerQml::devices() { // NOLINT
	return UPower::instance()->devices();
}

} // namespace qs::service::upower
