# cpp-transport-catalogue
## Финальный проект: транспортный справочник
### Описание:
Один из проектов, реализованный в ходе обучения в Яндекс Практикуме.

Позволяет осуществлять запросы на добавление остановок и маршрутов автобусов, просмотр информации об остановках и маршрутах. Вся информация сохраняется в структурах данных программы (словари, множества, массивы).

Реализована архитектура приложения с использованием полиморфизма. Поддерживает работу с запросами в различных форматах (собственный формат, JSON). 

Разработаны реализация парсера JSON и модуль построения графической карты маршрутов в формате SVG.

Работа над проектом позволила почувствовать, что есть еще порох в пороховницах (:
### Версия языка программирования: 
C++ 17
### Планы по доработке:
Разработать графический интерфейс пользователя.

Реализовать различные варианты DataLayer:
1. XML файлы.
2. База данных SQL.

### Развертывание:
1. Установить компилятор GCC
3. Распаковать код проекта в любую директорию в вашей операционной системе
4. С использованием командной строки перейти в данную директорию
5. Выполнить команду:
   
для Windows:
```
compile.cmd
```
для Linux:
```
  sudo chmod +x compile.sh
  sudo ./compile.sh
```
