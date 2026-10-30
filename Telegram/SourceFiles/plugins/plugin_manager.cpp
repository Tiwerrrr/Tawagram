/*
This file is part of Tawagram,
the desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "plugins/plugin_manager.h"

#include "core/application.h"
#include "core/core_settings.h"
#include "settings.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QRegularExpression>
#include <QtCore/QTextStream>
#include <QtWidgets/QApplication>

namespace Plugins {
namespace {

const auto kBasePluginCode = QString::fromUtf8(R"PYTHON(# exteraGram & Tawagram Plugin SDK
from typing import Any, Optional

class HookResult:
    def __init__(self, action: str, params: Any = None):
        self.action = action
        self.params = params

    @classmethod
    def next(cls):
        return cls("next")

    @classmethod
    def modify(cls, params: Any):
        return cls("modify", params)

    @classmethod
    def cancel(cls):
        return cls("cancel")

class HookStrategy:
    FIRST = 0
    LAST = 1
    CHAIN = 2

class BasePlugin:
    def __init__(self):
        self.logger = self

    def info(self, msg: str):
        print(f"[INFO] {msg}")

    def error(self, msg: str):
        print(f"[ERROR] {msg}")

    def on_plugin_load(self):
        pass

    def on_plugin_unload(self):
        pass

    def add_on_send_message_hook(self):
        pass

    def on_send_message_hook(self, account: int, params: Any) -> HookResult:
        return HookResult.next()
)PYTHON");

const auto kExampleHelloCode = QString::fromUtf8(R"PYTHON(from base_plugin import BasePlugin, HookResult
from typing import Any

__id__ = "hello_world"
__name__ = "Hello World & Shrug"
__description__ = "Replaces .hello with greetings and .shrug with ¯\\_(ツ)_/¯"
__author__ = "Tawagram Team"
__version__ = "1.0.0"

class HelloWorldPlugin(BasePlugin):
    def on_plugin_load(self):
        self.add_on_send_message_hook()

    def on_send_message_hook(self, account: int, params: Any) -> HookResult:
        msg = getattr(params, "message", "")
        if not isinstance(msg, str):
            return HookResult.next()

        if msg.strip() == ".shrug":
            params.message = "¯\\_(ツ)_/¯"
            return HookResult.modify(params)

        if msg.startswith(".hello"):
            parts = msg.split(maxsplit=1)
            name = parts[1] if len(parts) > 1 else "World"
            params.message = f"Hello, {name}!"
            return HookResult.modify(params)

        return HookResult.next()
)PYTHON");

const auto kExampleAppearanceCode = QString::fromUtf8(R"PYTHON(from base_plugin import BasePlugin

__id__ = "dark_accent_ui"
__name__ = "Custom Dark Accent Appearance"
__description__ = "Customizes Tawagram client appearance using QSS stylesheets"
__author__ = "Tawagram Team"
__version__ = "1.0.0"

# You can customize any Qt UI element of Tawagram here!
__stylesheet__ = """
/* Example appearance tweak: smooth rounded input fields and custom scrollbars */
QLineEdit, QTextEdit {
    border-radius: 8px;
}
QScrollBar:vertical {
    width: 8px;
    background: transparent;
}
QScrollBar::handle:vertical {
    border-radius: 4px;
}
"""

class AppearancePlugin(BasePlugin):
    def on_plugin_load(self):
        pass
)PYTHON");

} // namespace

Manager &Manager::Instance() {
	static Manager instance;
	return instance;
}

Manager::Manager() {
}

void Manager::init() {
	if (_initialized) {
		return;
	}
	_initialized = true;
	createDefaultPluginsIfEmpty();
	reload();
}

QString Manager::pluginsPath() const {
	const auto path = cWorkingDir() + u"tdata/plugins/"_q;
	QDir().mkpath(path);
	return path;
}

void Manager::createDefaultPluginsIfEmpty() {
	const auto dirPath = pluginsPath();
	const auto basePluginPath = dirPath + u"base_plugin.py"_q;
	if (!QFile::exists(basePluginPath)) {
		QFile file(basePluginPath);
		if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			QTextStream out(&file);
			out.setEncoding(QStringConverter::Utf8);
			out << kBasePluginCode;
		}
	}

	const auto helloPath = dirPath + u"hello_world.py"_q;
	if (!QFile::exists(helloPath)) {
		QFile file(helloPath);
		if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			QTextStream out(&file);
			out.setEncoding(QStringConverter::Utf8);
			out << kExampleHelloCode;
		}
	}

	const auto appearancePath = dirPath + u"appearance_theme.py"_q;
	if (!QFile::exists(appearancePath)) {
		QFile file(appearancePath);
		if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			QTextStream out(&file);
			out.setEncoding(QStringConverter::Utf8);
			out << kExampleAppearanceCode;
		}
	}
}

void Manager::reload() {
	scanPlugins();
	applyPluginStylesheets();
}

void Manager::scanPlugins() {
	_plugins.clear();
	const auto dirPath = pluginsPath();
	const auto dir = QDir(dirPath);
	const auto files = dir.entryInfoList({ u"*.py"_q, u"*.plugin"_q }, QDir::Files, QDir::Name);

	for (const auto &fileInfo : files) {
		if (fileInfo.fileName() == u"base_plugin.py"_q) {
			continue;
		}
		parsePluginFile(fileInfo.absoluteFilePath());
	}
}

void Manager::parsePluginFile(const QString &filePath) {
	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		return;
	}

	QTextStream in(&file);
	in.setEncoding(QStringConverter::Utf8);
	const auto content = in.readAll();

	PluginInfo info;
	info.filePath = filePath;

	const auto extract = [&](const QString &key) -> QString {
		const auto regex = QRegularExpression(
			key + u"\\s*=\\s*[\"'](.*?)[\"']"_q);
		const auto match = regex.match(content);
		return match.hasMatch() ? match.captured(1).trimmed() : QString();
	};

	info.id = extract(u"__id__"_q);
	if (info.id.isEmpty()) {
		info.id = QFileInfo(filePath).baseName();
	}
	info.name = extract(u"__name__"_q);
	if (info.name.isEmpty()) {
		info.name = info.id;
	}
	info.description = extract(u"__description__"_q);
	info.author = extract(u"__author__"_q);
	info.version = extract(u"__version__"_q);
	if (info.version.isEmpty()) {
		info.version = u"1.0.0"_q;
	}

	// Extract stylesheet if provided
	const auto cssRegex = QRegularExpression(
		u"__stylesheet__\\s*=\\s*\"\"\"(.*?)\"\"\""_q,
		QRegularExpression::DotMatchesEverythingOption);
	const auto cssMatch = cssRegex.match(content);
	if (cssMatch.hasMatch()) {
		info.stylesheet = cssMatch.captured(1).trimmed();
	}

	const auto prefKey = "plugin_enabled_" + info.id.toStdString();
	info.enabled = Core::App().settings().readPref<bool>(prefKey, true);

	_plugins.push_back(std::move(info));
}

const std::vector<PluginInfo> &Manager::plugins() const {
	return _plugins;
}

void Manager::setPluginEnabled(const QString &id, bool enabled) {
	for (auto &plugin : _plugins) {
		if (plugin.id == id) {
			plugin.enabled = enabled;
			const auto prefKey = "plugin_enabled_" + id.toStdString();
			Core::App().settings().writePref<bool>(prefKey, enabled);
			Core::App().saveSettingsDelayed();
			applyPluginStylesheets();
			break;
		}
	}
}

bool Manager::isPluginEnabled(const QString &id) const {
	for (const auto &plugin : _plugins) {
		if (plugin.id == id) {
			return plugin.enabled;
		}
	}
	return false;
}

void Manager::applyPluginStylesheets() {
	QString combined;
	for (const auto &plugin : _plugins) {
		if (plugin.enabled && !plugin.stylesheet.isEmpty()) {
			combined += u"\n/* Plugin: "_q + plugin.name + u" */\n"_q;
			combined += plugin.stylesheet + u"\n"_q;
		}
	}

	if (const auto app = QApplication::instance()) {
		app->setStyleSheet(combined);
	}
}

std::optional<QString> Manager::processOutgoingMessage(const QString &text) {
	if (text.isEmpty()) {
		return text;
	}

	auto result = text;
	// Process fast built-in hooks for active plugins
	for (const auto &plugin : _plugins) {
		if (!plugin.enabled) {
			continue;
		}

		// Built-in commands from hello_world plugin or custom shrug
		if (plugin.id == u"hello_world"_q) {
			if (result.trimmed() == u".shrug"_q) {
				result = u"¯\\_(ツ)_/¯"_q;
			} else if (result.startsWith(u".hello"_q)) {
				const auto parts = result.split(' ', Qt::SkipEmptyParts);
				const auto name = (parts.size() > 1) ? parts[1] : u"World"_q;
				result = u"Hello, "_q + name + u"!"_q;
			}
		}
	}

	return result;
}

} // namespace Plugins
