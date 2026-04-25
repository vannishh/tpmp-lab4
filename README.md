# Autopark — Система управления автопарком

> **Лабораторная работа №4** · Технологии программирования для мобильных приложений  
> Вариант 1 — «Автопарк»

---

## Project Name

**Autopark** — консольное приложение для управления автопарком с базой данных SQLite.

---

## Description

Приложение автоматизирует деятельность автопарка, осуществляющего обслуживание заказов
на перевозку грузов. Система обеспечивает:

- **Аутентификацию и авторизацию** пользователей по ролям (`admin` / `driver`).
- **CRUD-операции** над автомобилями, водителями и заказами.
- **Отчёты** по пробегу, грузообороту и заработку водителей.
- **Бизнес-логику**: контроль грузоподъёмности (триггер SQLite), расчёт заработной платы
  водителей (20 % от стоимости перевозки), сохранение результатов в `FLEET_EARNINGS`.

Хранение данных — **SQLite 3**. Язык — **C99**.

---

## Installation

```bash
# 1. Клонировать репозиторий
git clone https://github.com/vannishh/tpmp-lab4.git
cd autopark

# 2. Установить зависимости (Ubuntu / Debian)
sudo apt-get install libsqlite3-dev gcc make

# 3. Собрать проект
make

# 4. Запустить
./bin/tpmp-lab4
```

---

## Usage

При первом запуске база данных `autopark.db` создаётся автоматически.

**Учётные данные по умолчанию:**

| Роль   | Логин   | Пароль   |
|--------|---------|----------|
| Admin  | admin   | admin123 |
| Driver | ivanov  | pass123  |
| Driver | petrov  | pass123  |

**Меню администратора:**

```
=== Admin Panel ===
 1. Cars       — управление автомобилями
 2. Drivers    — управление водителями
 3. Orders     — заказы, отчёты, расчёт зарплат
 4. Register user
 0. Logout
```

**Меню водителя:**

```
=== Welcome, ivanov! ===
 1. My orders      — мои заказы
 2. My car info    — сведения о машине
 3. Change password
 0. Logout
```

---

## Running tests

```bash
make check       # запустить unit-тесты
make distcheck   # тесты + отчёт gcov
```

---

## Contributing

| Участник | Роль | Задачи |
|----------|------|--------|
| **Короткевич Дмитрий** | Team Lead | Архитектура, `db.c`, `auth.c`, `main.c`, Kanban, CI/CD, документация |
| **Близник Климентий** | Developer | `cars.c`, `drivers.c`, `orders.c`, unit-тесты, схема БД |

---

## Repository structure

```
autopark/
├── .github/workflows/c-cpp.yml   # GitHub Actions CI
├── bin/                           # Compiled binaries
├── build/                         # Object files
├── includes/                      # Header files
│   ├── db.h
│   ├── auth.h
│   ├── cars.h
│   ├── drivers.h
│   └── orders.h
├── src/                           # Source code
│   ├── main.c
│   ├── db.c
│   ├── auth.c
│   ├── cars.c
│   ├── drivers.c
│   └── orders.c
├── tests/                         # Unit tests
│   ├── test_cars.c
│   ├── test_drivers.c
│   └── test_orders.c
├── docs/
│   └── schema.sql                 # SQLite schema + seed data
├── Makefile
└── README.md
```
