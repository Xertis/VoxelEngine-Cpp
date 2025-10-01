# Библиотека *file*

Библиотека функций для работы с файлами

```python
file.resolve(путь: str) -> str
```

Функция приводит запись `точка_входа:путь` (например `user:worlds/house1`) к обычному пути. (например `C://Users/user/.voxeng/worlds/house1`)

> [!NOTE]
> Функцию не нужно использовать в сочетании с другими функциями из библиотеки, так как они делают это автоматически

Возвращаемый путь не является каноническим и может быть как абсолютным, так и относительным.

```python
file.read(путь: str) -> str
```

Читает весь текстовый файл и возвращает в виде строки

```python
file.read_bytes(путь: str, [опционально] usetable) -> array of integers
```

Читает файл в массив байт. При значении usetable = false возвращает Bytearray вместо table.

```lua
file.is_writeable(путь: str) -> bool
```

Проверяет, доступно ли право записи по указанному пути.

```python
file.write(путь: str, текст: str) -> nil
```

Записывает текст в файл (с перезаписью)

```python
file.write_bytes(путь: str, data: array of integers)
```

Записывает массив байт в файл (с перезаписью)

```python
file.gzip_compress(data: array of integers, [опционально] usetable) -> array of integers
```

Сжимает массив байт алгоритмом gzip. При значении usetable = false возвращает Bytearray вместо table.

```python
file.gzip_decompress(data: array of integers, [опционально] usetable) -> array of integers
```

Разжимает массив байт, который был сжат алгоритмом gzip. При значении usetable = false возвращает Bytearray вместо table.

```python
file.length(путь: str) -> int
```

Возвращает размер файла в байтах, либо -1, если файл не найден

```python
file.exists(путь: str) -> bool
```

Проверяет, существует ли по данному пути файл или директория

```python
file.isfile(путь: str) -> bool
```

Проверяет, существует ли по данному пути файл

```python
file.isdir(путь: str) -> bool
```

Проверяет, существует ли по данному пути директория

```python
file.mkdir(путь: str) -> bool
```

Создает директорию. Возвращает true если была создана новая директория

```python
file.mkdirs(путь: str) -> bool
```

Создает всю цепочку директорий. Возвращает true если были созданы директории.

```python
file.list(путь: str) -> массив строк
```

Возвращает список файлов и директорий в указанной.

```python
file.list_all_res(путь: str) -> массив строк
```

Возвращает список файлов и директорий в указанной без указания конкретной точки входа.

```python
file.find(путь: str) -> str
```

Ищет файл от последнего пака до res. Путь указывается без префикса. Возвращает путь с нужным префиксом. Если файл не найден, возвращает nil.

```python
file.remove(путь: str) -> bool
```

Удаляет файл. Возращает **true** если файл существовал. Бросает исключение при нарушении доступа.

```python
file.remove_tree(путь: str) -> int
```

Рекурсивно удаляет файлы. Возвращает число удаленных файлов.

```python
file.read_combined_list(путь: str) -> массив
```

Совмещает массивы из JSON файлов разных паков.

```python
file.read_combined_object(путь: str) -> массив
```

Совмещает объекты из JSON файлов разных паков.

```lua
file.mount(путь: str) --> str
```

Монтирует ZIP-архив как файловой системе. Возвращает имя точки входа.

```lua
file.unmount(точка_входа: str) --> str
```

Размонтирует точку входа.

```lua
file.create_zip(директория: str, выходной_файл: str) --> str
```

Создаёт ZIP-архив из содержимого указанной директории.

```lua
file.name(путь: str) --> str
```

Извлекает имя файла из пути. Пример: `world:data/base/config.toml` -> `config.toml`.

```lua
file.stem(путь: str) --> str
```

Извлекает имя файла из пути, удаляя расширение. Пример: `world:data/base/config.toml` -> `config`.

```lua
file.ext(путь: str) --> str
```

Извлекает расширение из пути. Пример: `world:data/base/config.toml` -> `toml`.

```lua
file.prefix(путь: str) --> str
```

Извлекает точку входа (префикс) из пути. Пример: `world:data/base/config.toml` -> `world`.

```lua
file.parent(путь: str) --> str
```

Возвращает путь на уровень выше. Пример: `world:data/base/config.toml` -> `world:data/base`

```lua
file.path(путь: str) --> str
```

Убирает точку входа (префикс) из пути. Пример: `world:data/base/config.toml` -> `data/base/config.toml`


```lua
file.join(директория: str, путь: str) --> str
```

Соединяет путь. Пример: `file.join("world:data", "base/config.toml)` -> `world:data/base/config.toml`.

Следует использовать данную функцию вместо конкатенации с `/`, так как `префикс:/путь` не является валидным.

```lua
file.open(путь: str, режим: str) --> io_stream
```

Открывает поток для записи/чтения в файл по пути `путь`.

Аргумент `режим` это список отдельных режимов, в котором каждый обозначается одним символом

`r` - Чтение из файла
`w` - Запись в файл
`b` - Открыть поток в двоичном режиме (см. `../io_stream.md`)
`+` - Работает совместно с `w`. Добавляет к существующим данным новые (`append-mode`)

```lua
file.open_named_pipe(имя: str, режим: str) -> io_stream
```

Открывает поток для записи/чтения в Named Pipe по пути `путь`

`/tmp/` или `\\\\.\\pipe\\` добавлять не нужно - движок делает это автоматически.

Доступные режимы такие же, как и в `file.open`, за исключением `+`
