-- ============================================================
-- Autopark Database Schema — SQLite
-- Variant 1: «Автопарк»
-- Team: Короткевич Дмитрий & Близник Климентий
-- ============================================================

PRAGMA foreign_keys = ON;

-- Users table (authentication & authorization)
CREATE TABLE IF NOT EXISTS FLEET_USERS (
    user_id   INTEGER PRIMARY KEY AUTOINCREMENT,
    username  TEXT    NOT NULL UNIQUE,
    password  TEXT    NOT NULL,
    role      TEXT    NOT NULL CHECK(role IN ('admin', 'driver')),
    driver_id INTEGER DEFAULT 0,
    FOREIGN KEY (driver_id) REFERENCES FLEET_DRIVERS(driver_id)
);

-- Cars table
CREATE TABLE IF NOT EXISTS FLEET_CARS (
    car_id          INTEGER PRIMARY KEY AUTOINCREMENT,
    license_plate   TEXT    NOT NULL UNIQUE,
    brand           TEXT    NOT NULL,
    initial_mileage INTEGER NOT NULL DEFAULT 0,
    capacity        REAL    NOT NULL   -- max cargo in tons
);

-- Drivers table
CREATE TABLE IF NOT EXISTS FLEET_DRIVERS (
    driver_id   INTEGER PRIMARY KEY AUTOINCREMENT,
    employee_id TEXT    NOT NULL UNIQUE,
    last_name   TEXT    NOT NULL,
    category    TEXT    NOT NULL,
    experience  INTEGER NOT NULL,
    address     TEXT,
    birth_year  INTEGER NOT NULL
);

-- Orders table
CREATE TABLE IF NOT EXISTS FLEET_ORDERS (
    order_id       INTEGER PRIMARY KEY AUTOINCREMENT,
    order_date     DATE    NOT NULL,
    driver_id      INTEGER NOT NULL,
    car_id         INTEGER NOT NULL,
    distance_km    REAL    NOT NULL,
    cargo_weight   REAL    NOT NULL,  -- tons
    transport_cost REAL    NOT NULL,
    FOREIGN KEY (driver_id) REFERENCES FLEET_DRIVERS(driver_id),
    FOREIGN KEY (car_id)    REFERENCES FLEET_CARS(car_id)
);

-- Earnings summary (populated by business-logic function)
CREATE TABLE IF NOT EXISTS FLEET_EARNINGS (
    earnings_id    INTEGER PRIMARY KEY AUTOINCREMENT,
    driver_id      INTEGER NOT NULL,
    period_start   DATE    NOT NULL,
    period_end     DATE    NOT NULL,
    total_earnings REAL    NOT NULL,
    FOREIGN KEY (driver_id) REFERENCES FLEET_DRIVERS(driver_id)
);

-- ── Trigger: prevent insert if cargo > car capacity ─────────
CREATE TRIGGER IF NOT EXISTS trg_check_cargo
BEFORE INSERT ON FLEET_ORDERS
BEGIN
    SELECT CASE
        WHEN NEW.cargo_weight > (
            SELECT capacity FROM FLEET_CARS WHERE car_id = NEW.car_id
        )
        THEN RAISE(ABORT, 'Cargo weight exceeds vehicle capacity')
    END;
END;

-- ── Seed data ────────────────────────────────────────────────
INSERT OR IGNORE INTO FLEET_USERS(username, password, role)
    VALUES ('admin', 'admin123', 'admin');

INSERT OR IGNORE INTO FLEET_CARS(license_plate, brand, initial_mileage, capacity)
VALUES
    ('АА1234ВВ', 'Volvo FH', 15000, 20.0),
    ('ВВ5678СС', 'MAN TGX',  8000,  18.0),
    ('СС9012ДД', 'Камаз 5490', 22000, 15.0),
    ('ДД3456ЕЕ', 'ГАЗ Газон', 5000,  5.0);

INSERT OR IGNORE INTO FLEET_DRIVERS
    (employee_id, last_name, category, experience, address, birth_year)
VALUES
    ('EMP001', 'Иванов',   'CE', 12, 'Минск, ул. Ленина 1',   1982),
    ('EMP002', 'Петров',   'C',   8, 'Минск, ул. Мира 5',     1987),
    ('EMP003', 'Сидоров',  'CE', 15, 'Гродно, ул. Советская', 1978),
    ('EMP004', 'Козлов',   'B',   3, 'Брест, пр. Пушкина 12', 1996);

INSERT OR IGNORE INTO FLEET_USERS(username, password, role, driver_id)
VALUES
    ('ivanov',  'pass123', 'driver', 1),
    ('petrov',  'pass123', 'driver', 2),
    ('sidorov', 'pass123', 'driver', 3),
    ('kozlov',  'pass123', 'driver', 4);

INSERT OR IGNORE INTO FLEET_ORDERS
    (order_date, driver_id, car_id, distance_km, cargo_weight, transport_cost)
VALUES
    ('2025-01-10', 1, 1, 450.0, 18.5, 12500.00),
    ('2025-01-15', 2, 2, 300.0, 15.0,  9800.00),
    ('2025-02-03', 1, 1, 520.0, 19.0, 14200.00),
    ('2025-02-18', 3, 3, 180.0, 12.0,  6500.00),
    ('2025-03-05', 4, 4,  90.0,  4.5,  3200.00),
    ('2025-03-12', 2, 2, 410.0, 17.0, 11300.00),
    ('2025-03-25', 1, 1, 600.0, 20.0, 17500.00);
