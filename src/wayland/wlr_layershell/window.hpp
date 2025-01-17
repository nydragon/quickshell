#pragma once

#include <qobject.h>
#include <qscreen.h>
#include <qtclasshelpermacros.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <qwindow.h>

#include "../../window/panelinterface.hpp"

///! WlrLayershell layer.
/// See @@WlrLayershell.layer.
namespace WlrLayer { // NOLINT
Q_NAMESPACE;
QML_ELEMENT;

enum Enum : quint8 {
	/// Below bottom
	Background = 0,
	/// Above background, usually below windows
	Bottom = 1,
	/// Commonly used for panels, app launchers, and docks.
	/// Usually renders over normal windows and below fullscreen windows.
	Top = 2,
	/// Usually renders over fullscreen windows
	Overlay = 3,
};
Q_ENUM_NS(Enum);

} // namespace WlrLayer

///! WlrLayershell keyboard focus mode
/// See @@WlrLayershell.keyboardFocus.
namespace WlrKeyboardFocus { // NOLINT
Q_NAMESPACE;
QML_ELEMENT;

enum Enum : quint8 {
	/// No keyboard input will be accepted.
	None = 0,
	/// Exclusive access to the keyboard, locking out all other windows.
	///
	/// > [!WARNING] You **CANNOT** use this to make a secure lock screen.
	/// >
	/// > If you want to make a lock screen, use @@WlSessionLock.
	Exclusive = 1,
	/// Access to the keyboard as determined by the operating system.
	///
	/// > [!WARNING] On some systems, `OnDemand` may cause the shell window to
	/// > retain focus over another window unexpectedly.
	/// > You should try `None` if you experience issues.
	OnDemand = 2,
};
Q_ENUM_NS(Enum);

} // namespace WlrKeyboardFocus

class QSWaylandLayerSurface;

class LayershellWindowExtension: public QObject {
	Q_OBJECT;

public:
	LayershellWindowExtension(QObject* parent = nullptr): QObject(parent) {}
	~LayershellWindowExtension() override;
	Q_DISABLE_COPY_MOVE(LayershellWindowExtension);

	// returns the layershell extension if attached, otherwise nullptr
	static LayershellWindowExtension* get(QWindow* window);

	// Attach this layershell extension to the given window.
	// The extension is reparented to the window and replaces any existing layershell extension.
	// Returns false if the window cannot be used.
	bool attach(QWindow* window);

	void setAnchors(Anchors anchors);
	[[nodiscard]] Anchors anchors() const;

	void setMargins(Margins margins);
	[[nodiscard]] Margins margins() const;

	void setExclusiveZone(qint32 exclusiveZone);
	[[nodiscard]] qint32 exclusiveZone() const;

	void setLayer(WlrLayer::Enum layer);
	[[nodiscard]] WlrLayer::Enum layer() const;

	void setKeyboardFocus(WlrKeyboardFocus::Enum focus);
	[[nodiscard]] WlrKeyboardFocus::Enum keyboardFocus() const;

	// no effect if configured
	void setUseWindowScreen(bool value);
	void setNamespace(QString ns);
	[[nodiscard]] QString ns() const;
	[[nodiscard]] bool isConfigured() const;

signals:
	void anchorsChanged();
	void marginsChanged();
	void exclusiveZoneChanged();
	void layerChanged();
	void keyboardFocusChanged();

private:
	// if configured the screen cannot be changed
	QSWaylandLayerSurface* surface = nullptr;

	bool useWindowScreen = false;
	Anchors mAnchors;
	Margins mMargins;
	qint32 mExclusiveZone = 0;
	WlrLayer::Enum mLayer = WlrLayer::Top;
	QString mNamespace = "quickshell";
	WlrKeyboardFocus::Enum mKeyboardFocus = WlrKeyboardFocus::None;

	friend class QSWaylandLayerSurface;
};
