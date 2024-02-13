#include "proxywindow.hpp"

#include <qobject.h>
#include <qqmllist.h>
#include <qquickitem.h>
#include <qquickwindow.h>
#include <qtypes.h>
#include <qwindow.h>

ProxyWindowBase::~ProxyWindowBase() {
	if (this->window != nullptr) {
		this->window->deleteLater();
	}
}

void ProxyWindowBase::earlyInit(QObject* old) {
	auto* oldpw = qobject_cast<ProxyWindowBase*>(old);

	if (oldpw == nullptr || oldpw->window == nullptr) {
		this->window = new QQuickWindow();
	} else {
		this->window = oldpw->disownWindow();
	}

	// clang-format off
	QObject::connect(this->window, &QWindow::visibilityChanged, this, &ProxyWindowBase::visibleChanged);
	QObject::connect(this->window, &QWindow::widthChanged, this, &ProxyWindowBase::widthChanged);
	QObject::connect(this->window, &QWindow::heightChanged, this, &ProxyWindowBase::heightChanged);
	QObject::connect(this->window, &QQuickWindow::colorChanged, this, &ProxyWindowBase::colorChanged);
	// clang-format on
}

QQuickWindow* ProxyWindowBase::disownWindow() {
	QObject::disconnect(this->window, nullptr, this, nullptr);

	auto data = this->data();
	ProxyWindowBase::dataClear(&data);
	data.clear(&data);

	auto* window = this->window;
	this->window = nullptr;
	return window;
}

QQuickWindow* ProxyWindowBase::backingWindow() { return this->window; }
QQuickItem* ProxyWindowBase::item() { return this->window->contentItem(); }

// NOLINTNEXTLINE
#define PROXYPROP(type, get, set)                                                                  \
	type ProxyWindowBase::get() { return this->window->get(); }                                      \
	void ProxyWindowBase::set(type value) { this->window->set(value); }

PROXYPROP(bool, isVisible, setVisible);
PROXYPROP(qint32, width, setWidth);
PROXYPROP(qint32, height, setHeight);
PROXYPROP(QColor, color, setColor);

// see:
// https://code.qt.io/cgit/qt/qtdeclarative.git/tree/src/quick/items/qquickwindow.cpp
// https://code.qt.io/cgit/qt/qtdeclarative.git/tree/src/quick/items/qquickitem.cpp
//
// relevant functions are private so we call them via the property

QQmlListProperty<QObject> ProxyWindowBase::data() {
	return QQmlListProperty<QObject>(
	    this,
	    nullptr,
	    ProxyWindowBase::dataAppend,
	    ProxyWindowBase::dataCount,
	    ProxyWindowBase::dataAt,
	    ProxyWindowBase::dataClear,
	    ProxyWindowBase::dataReplace,
	    ProxyWindowBase::dataRemoveLast
	);
}

QQmlListProperty<QObject> ProxyWindowBase::dataBacker(QQmlListProperty<QObject>* prop) {
	auto* that = static_cast<ProxyWindowBase*>(prop->object); // NOLINT
	return that->window->property("data").value<QQmlListProperty<QObject>>();
}

void ProxyWindowBase::dataAppend(QQmlListProperty<QObject>* prop, QObject* obj) {
	auto backer = dataBacker(prop);
	backer.append(&backer, obj);
}

qsizetype ProxyWindowBase::dataCount(QQmlListProperty<QObject>* prop) {
	auto backer = dataBacker(prop);
	return backer.count(&backer);
}

QObject* ProxyWindowBase::dataAt(QQmlListProperty<QObject>* prop, qsizetype i) {
	auto backer = dataBacker(prop);
	return backer.at(&backer, i);
}

void ProxyWindowBase::dataClear(QQmlListProperty<QObject>* prop) {
	auto backer = dataBacker(prop);
	backer.clear(&backer);
}

void ProxyWindowBase::dataReplace(QQmlListProperty<QObject>* prop, qsizetype i, QObject* obj) {
	auto backer = dataBacker(prop);
	backer.replace(&backer, i, obj);
}

void ProxyWindowBase::dataRemoveLast(QQmlListProperty<QObject>* prop) {
	auto backer = dataBacker(prop);
	backer.removeLast(&backer);
}

void ProxyFloatingWindow::earlyInit(QObject* old) {
	this->ProxyWindowBase::earlyInit(old);
	this->geometryLocked = this->window->isVisible();
}

void ProxyFloatingWindow::componentComplete() {
	this->ProxyWindowBase::componentComplete();
	this->geometryLocked = true;
}

void ProxyFloatingWindow::setWidth(qint32 value) {
	if (!this->geometryLocked) {
		this->ProxyWindowBase::setWidth(value);
	}
}

void ProxyFloatingWindow::setHeight(qint32 value) {
	if (!this->geometryLocked) {
		this->ProxyWindowBase::setHeight(value);
	}
}
