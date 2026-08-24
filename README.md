# v8unpack

Консольная утилита для разбора и обратной сборки контейнеров 1С (`.cf`,
`.epf`, `.erf`), а также упаковки и распаковки их внутренних блоков.

Это поддерживаемая сборка форка v8Unpack: **Built by Cujoko**. Проект основан
на работах Denis Demidov, Sergey Batanov, Sergey Rudakov и других участников.

## Авторы и участники

- Denis Demidov;
- Sergey Batanov;
- Sergey Rudakov;
- Cujoko — развитие и сопровождение этого форка, современная сборка Windows
  и улучшения производительности `parse/build`.

## Скачать

Готовые выпуски Windows x64 находятся в
[GitHub Releases](https://github.com/cujoko/v8unpack/releases). Portable-архив
имеет имя `v8unpack-<version>-win-x64-built-by-cujoko.zip` и содержит:

- `v8unpack.exe`;
- `README.md` и `LICENSE`;
- `SHA256SUMS.txt` с контрольной суммой исполняемого файла.

## Основные команды

```text
v8unpack parse    input.epf output-directory [block-name ...]
v8unpack build    input-directory output.epf
```

## Диагностика

```text
v8unpack check    input.epf [--json]
v8unpack info     input.epf [--json]
v8unpack manifest input.epf manifest.json
v8unpack -listfiles input.epf
```

## Низкоуровневые команды

```text
v8unpack unpack   input.cf output-directory [block-name]
v8unpack pack     input-directory output.cf
v8unpack inflate  input.data output
v8unpack deflate  input output.data
```

`parse/build` предназначены для обычного рекурсивного разбора и сборки.
При `build` крупные файлы параллельно сжимаются во временный каталог, поэтому
на время сборки требуется дополнительное свободное место примерно до размера
выходного контейнера. Временные файлы удаляются по мере записи результата.
`unpack/pack` сохраняют низкоуровневое представление из `FileHeader`, `.header`
и `.data`, а `inflate/deflate` работают непосредственно с потоками zlib.

Старые ключи (`-P`, `-B`, `-U`, `-PA`, `-I`, `-D`, `-LF`) сохранены для
совместимости. Общие параметры: `--force`, `--quiet`, `--verbose`, `--json`.
Без `--force` существующий выходной файл или каталог не перезаписывается.

## Сборка Windows x64

Нужны CMake 3.24+, Visual Studio 2022 с C++ workload и vcpkg. Переменная
`VCPKG_ROOT` должна указывать на каталог vcpkg.

```powershell
cmake --preset windows-x64
cmake --build --preset windows-x64 --parallel
ctest --preset windows-x64
```

Зависимость zlib описана в `vcpkg.json`; Boost больше не требуется. Сборка
использует C++17 и статическую MSVC runtime, поэтому итоговый EXE переносим.

## Тестовые файлы

Синтетический round-trip выполняется всегда. Личные `.epf`, `.cf` и другие
крупные образцы можно класть в `test/local-fixtures/`: каталог находится в
`.gitignore`. Если там есть `cjk-evotor-settings-util.epf`, CTest дополнительно
проверяет его точный `parse → build` round-trip по SHA-256.

## Выпуск версии

1. Обновить версию в `project(... VERSION ...)` и `vcpkg.json`.
2. Собрать и выполнить тесты.
3. Создать и отправить трёхкомпонентный SemVer-тег, например `3.2.0`.

Публичная версия, Git-тег, CLI и имя ZIP используют три компонента (`3.2.0`).
Числовые свойства Windows EXE автоматически дополняются четвёртым нулём
(`3.2.0.0`).

GitHub Actions проверит совпадение тега с версией проекта, соберёт Windows x64,
запустит тесты, создаст ZIP и опубликует GitHub Release. CI запускается для
pull request в `dev` и для push в `main`; обычный push в рабочую ветку `dev`
проверку не запускает.

## Лицензия

[Mozilla Public License 2.0](LICENSE).
