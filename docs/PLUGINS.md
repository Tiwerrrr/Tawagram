# Руководство по плагинам Tawagram (Tawagram Plugins SDK)

Tawagram поддерживает систему плагинов, вдохновлённую форматом и архитектурой **exteraGram**. Плагины пишутся на языке Python (`.py`) и позволяют модифицировать поведение клиента, перехватывать сообщения, добавлять команды и менять внешний вид интерфейса.

---

## 📁 Где находятся плагины

Все плагины хранятся в папке:
```text
tdata/plugins/
```
*(Папка создается автоматически при первом запуске клиента. Вы также можете открыть её прямо из клиента: **Настройки → Конфиденциальность → Открыть папку с плагинами**).*

---

## 🚀 Базовая структура плагина

Каждый плагин представляет собой файл с расширением `.py` (например, `my_plugin.py`). Он должен наследовать базовый класс `BasePlugin` из модуля `base_plugin`.

```python
from base_plugin import BasePlugin, HookResult
from typing import Any

# Обязательные метаданные плагина
__id__ = "my_custom_plugin"
__name__ = "Мой первый плагин"
__description__ = "Описание того, что делает плагин"
__author__ = "Ваше имя / никнейм"
__version__ = "1.0.0"

class MyPlugin(BasePlugin):
    def on_plugin_load(self):
        """Вызывается при загрузке плагина клиентом."""
        self.add_on_send_message_hook()
        self.logger.info("Плагин успешно загружен!")

    def on_plugin_unload(self):
        """Вызывается при отключении или выгрузке плагина."""
        pass

    def on_send_message_hook(self, account: int, params: Any) -> HookResult:
        """
        Перехват отправки исходящего сообщения.
        Позволяет изменить текст, отменить отправку или пропустить.
        """
        msg = getattr(params, "message", "")
        if not isinstance(msg, str):
            return HookResult.next()

        # Пример: быстрая замена команды .shrug
        if msg.strip() == ".shrug":
            params.message = "¯\\_(ツ)_/¯"
            return HookResult.modify(params)

        return HookResult.next()
```

---

## 🎨 Изменение внешнего вида клиента (Appearance & Themes)

В Tawagram плагины могут менять оформление любых элементов интерфейса (кнопок, полей ввода, скроллбаров, панелей, диалогов) с помощью **Qt Style Sheets (QSS / CSS)**.

Для этого в начале плагина достаточно объявить переменную `__stylesheet__`:

```python
from base_plugin import BasePlugin

__id__ = "cyberpunk_neon_theme"
__name__ = "Киберпанк Тема"
__description__ = "Стильный тёмный интерфейс с неоновыми акцентами и скруглёнными элементами"
__author__ = "Tawagram Designer"
__version__ = "1.1.0"

__stylesheet__ = """
/* Скругление полей ввода сообщений и поиска */
QLineEdit, QTextEdit {
    border-radius: 10px;
    padding: 6px 10px;
    background-color: #1e1f29;
    color: #f8f8f2;
    selection-background-color: #bd93f9;
}

/* Красивые тонкие скроллбары */
QScrollBar:vertical {
    border: none;
    background: transparent;
    width: 6px;
    margin: 0px;
}
QScrollBar::handle:vertical {
    background: #44475a;
    min-height: 20px;
    border-radius: 3px;
}
QScrollBar::handle:vertical:hover {
    background: #6272a4;
}
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0px;
}

/* Стилизация всплывающих подсказок */
QToolTip {
    background-color: #282a36;
    color: #50fa7b;
    border: 1px solid #6272a4;
    border-radius: 6px;
    padding: 4px 8px;
}
"""

class CyberpunkThemePlugin(BasePlugin):
    def on_plugin_load(self):
        self.logger.info("Киберпанк оформление активировано")
```

---

## 🛠️ Справочник API (exteraGram Compatible)

### Класс `HookResult`
Определяет результат обработки события хуком:
* `HookResult.next()` — передать управление следующему плагину без изменений.
* `HookResult.modify(params)` — применить изменения (например, изменённый текст `params.message`).
* `HookResult.cancel()` — отменить действие (например, прервать отправку сообщения).

### События жизненного цикла
* `on_plugin_load(self)` — инициализация плагина.
* `on_plugin_unload(self)` — деинициализация и очистка.
* `add_on_send_message_hook(self)` — регистрация перехватчика отправки сообщений.
* `on_send_message_hook(self, account: int, params: Any)` — вызывается перед отправкой сообщения в любой чат.

---

## 💡 Примеры готовых плагинов

### 1. Плагин быстрых команд (`text_expander.py`)
```python
from base_plugin import BasePlugin, HookResult
from typing import Any
import datetime

__id__ = "text_expander"
__name__ = "Text Expander & Utilities"
__description__ = "Полезные команды: .time, .date, .calc, .lenny"
__author__ = "Community"
__version__ = "1.0.0"

class TextExpanderPlugin(BasePlugin):
    def on_plugin_load(self):
        self.add_on_send_message_hook()

    def on_send_message_hook(self, account: int, params: Any) -> HookResult:
        text = getattr(params, "message", "")
        if not text:
            return HookResult.next()

        if text.strip() == ".time":
            params.message = f"Текущее время: {datetime.datetime.now().strftime('%H:%M:%S')}"
            return HookResult.modify(params)

        if text.strip() == ".lenny":
            params.message = "( ͡° ͜ʖ ͡°)"
            return HookResult.modify(params)

        if text.startswith(".calc "):
            expr = text[6:].strip()
            try:
                # Безопасное вычисление базовой математики
                res = eval(expr, {"__builtins__": None}, {})
                params.message = f"{expr} = {res}"
                return HookResult.modify(params)
            except Exception:
                pass

        return HookResult.next()
```

---

## ⚙️ Управление плагинами в интерфейсе клиента

В настройках Tawagram перейдите в:
**Настройки → Конфиденциальность → Плагины (Tawagram Plugins)**

Здесь доступны:
1. Кнопка **«Открыть папку с плагинами»** — мгновенно открывает проводник с папкой `tdata/plugins/`. Вы можете просто перетащить туда скачанный `.py` файл.
2. Кнопка **«Перезагрузить плагины»** — перечитывает файлы без перезапуска всего Telegram.
3. Список всех обнаруженных плагинов с их названиями, описанием, версией и тумблером для быстрого включения/отключения.
