/*
This file is part of Tawagram,
the desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include <QtCore/QString>
#include <vector>
#include <optional>

namespace Plugins {

struct PluginInfo {
	QString id;
	QString name;
	QString description;
	QString author;
	QString version;
	QString filePath;
	QString stylesheet;
	bool enabled = true;
};

class Manager final {
public:
	static Manager &Instance();

	void init();
	void reload();

	[[nodiscard]] const std::vector<PluginInfo> &plugins() const;
	void setPluginEnabled(const QString &id, bool enabled);
	[[nodiscard]] bool isPluginEnabled(const QString &id) const;
	[[nodiscard]] QString pluginsPath() const;

	// Returns std::nullopt if the message sending should be cancelled,
	// or the resulting (possibly modified) text string.
	[[nodiscard]] std::optional<QString> processOutgoingMessage(const QString &text);

	void applyPluginStylesheets();

private:
	Manager();
	~Manager() = default;
	Manager(const Manager &) = delete;
	Manager &operator=(const Manager &) = delete;

	void scanPlugins();
	void parsePluginFile(const QString &filePath);
	void createDefaultPluginsIfEmpty();

	std::vector<PluginInfo> _plugins;
	bool _initialized = false;
};

} // namespace Plugins
