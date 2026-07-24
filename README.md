# v8Unpack, GCC edition

Добавлен проект CodeBlocks 13.12.
Для сборки требуется Boost.

## Fork of v8Unpack project by Denis Demidov (disa_da2@mail.ru)

[Original project HOME](https://www.assembla.com/spaces/V8Unpack/team)

[Original project svn repo](http://svn2.assembla.com/svn/V8Unpack/)

## Note

V8Unpack - a small console program for rebuild/build configuration files [1C](http://1c.ru) such as _.cf _.epf \*.erf

## Plaform

Windows, POSIX

## Download

Готовые выпуски для Windows x64 доступны на странице
[GitHub Releases](https://github.com/Cujoko/v8unpack/releases).

Релизные файлы имеют имя `v8unpack-<version>-win-x64-built-by-cujoko.exe`.

## Build

Для сборки из исходного кода требуются компилятор с поддержкой C++14, CMake,
Boost (`filesystem`, `system`, `iostreams`) и Zlib.

## Version 3.0

- Оптимизирована сборка .cf файла ключ -B[UILD]. В версии 2.0 сборка корневого контейнера происходила в оперативной памяти.
  При сборке больших конфигураций это могло приводить к ошибке "segmentation fault". В версии 3.0 сборка корневого контейнера происходит
  динамически с сохранением элементов контейнера непосредственно в файл по мере их создания.

## Использование

```
  -U[NPACK]            in_filename.cf     out_dirname
  -U[NPACK]  -L[IST]   listfile
  -PA[CK]              in_dirname         out_filename.cf
  -PA[CK]    -L[IST]   listfile
  -I[NFLATE]           in_filename.data   out_filename
  -I[NFLATE] -L[IST]   listfile
  -D[EFLATE]           in_filename        filename.data
  -D[EFLATE] -L[IST]   listfile
  -P[ARSE]             in_filename        out_dirname
  -P[ARSE]   -L[IST]   listfile
  -B[UILD] [-N[OPACK]] in_dirname         out_filename
  -B[UILD] [-N[OPACK]] -L[IST] listfile
```
