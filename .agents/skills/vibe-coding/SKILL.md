---
name: vibe-coding
description: Fast, intuitive, and safe development of Tawagram features, plugins, UI theming, and Telegram customizations using natural language with AI. Use when asking how to build features quickly, create Python plugins, customize Telegram appearance, navigate the codebase, or guide an AI agent on modifying Tawagram without breaking builds or serialization.
---

# Vibe-Coding in Tawagram

Vibe-coding is the art of building software at the speed of thought using AI. Instead of memorizing thousands of lines of C++ boilerplate, Qt internals, or MTProto schemas, you describe what you want in plain human language, and the AI handles the implementation.

This skill equips any AI assistant with deep contextual knowledge of the **Tawagram** codebase to safely implement features, create plugins, and tweak UI without breaking builds or data serialization.

---

## The Tawagram Architectural Cheat Sheet

When a user asks to add or change something, route to the right layer:

| Goal / Feature | Where to Implement | Key Functions / Files |
|---|---|---|
| **Quick command / text shortcut** | Python Plugin (`tdata/plugins/*.py`) | `on_send_message_hook`, `params.message` |
| **Change UI theme / colors / fonts** | Python Plugin or QSS | `__stylesheet__ = """ ... """` |
| **Message intercept / rewriting** | `Telegram/SourceFiles/history/history_widget.cpp` | `sendTextWithTags()` |
| **Online presence / Ghost mode** | `Telegram/SourceFiles/api/api_updates.cpp` | `Updates::updateOnline()` |
| **Ghost reading / Read receipts** | `Telegram/SourceFiles/data/data_histories.cpp` | `Histories::sendReadRequest()` |
| **Anti-delete / Message persistence** | `Telegram/SourceFiles/data/data_session.cpp` | `processMessagesDeleted()` |
| **Date & status footer badges** | `Telegram/SourceFiles/history/view/history_view_bottom_info.cpp` | `layoutDateText()`, `Flag` |
| **View-once / Expiring media (TTL)** | `Telegram/SourceFiles/data/data_media_types.cpp` | `ttlSeconds()`, `ttlSecondsSingleView()` |
| **Settings switches & inputs** | `Telegram/SourceFiles/settings/sections/settings_privacy_security.cpp` | `SectionBuilder::addButton()` |
| **Storing custom settings** | `Core::App().settings()` | `readPref<Type>()`, `writePref<Type>()` |

---

## 3 Ironclad Rules for AI Vibe-Coding in Tawagram

1. **Never break settings serialization**:
   - DO NOT insert fields into the binary `QDataStream` of `Core::Settings` or `SessionSettings`.
   - ALWAYS use `Core::App().settings().readPref<Type>("key", default_val)` and `writePref<Type>("key", val)`. It is safe, KV-based, and never corrupts user profiles.
2. **Keep line endings consistent**:
   - On Windows, always preserve CRLF line endings.
   - Never save UTF-8 files with a BOM (Byte Order Mark). Always use UTF-8 without BOM.
3. **Prefer Python plugins for lightweight features**:
   - If a feature can be done with a plugin (`.py` in `tdata/plugins/`), do it as a plugin! It avoids long recompilation times and users can share or toggle it in one click.

---

## Ready-to-Use Vibe Prompts

Copy-paste these prompts when asking an AI assistant to build something new:

### 1. New Command / Automation:
> "Сделай плагин для Tawagram, который перехватывает команду `.weather <город>` и заменяет её на текущую погоду / заменяет команду `.r <текст>` на спойлер."

### 2. Custom UI Styling:
> "Напиши плагин темы для Tawagram, который делает интерфейс в стиле Gruvbox/Dracula, со скруглёнными аватарками и кастомными градиентными скроллбарами через `__stylesheet__`."

### 3. New Client Setting:
> "Добавь в настройки Tawagram тумблер 'Скрывать плашку спонсированных сообщений' с сохранением через `readPref/writePref` и примени фильтрацию в `history_view_bottom_info.cpp`."
