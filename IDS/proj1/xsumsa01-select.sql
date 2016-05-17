-- vi:syntax=plsql
SET sqlblanklines ON;
SET linesize 150;

-- TODO
DROP TABLE pobocka CASCADE CONSTRAINTS;
DROP TABLE darce CASCADE CONSTRAINTS;
DROP TABLE klient CASCADE CONSTRAINTS;
DROP TABLE pozvanka CASCADE CONSTRAINTS;
DROP TABLE vzorek CASCADE CONSTRAINTS;
DROP TABLE odber CASCADE CONSTRAINTS;
DROP TABLE davka CASCADE CONSTRAINTS;
DROP TABLE rezervace CASCADE CONSTRAINTS;

CREATE TABLE pobocka
(
    id      NUMBER GENERATED ALWAYS AS IDENTITY,
    nazev   VARCHAR2(255) NOT NULL,
    adresa  VARCHAR2(512) NOT NULL,

    PRIMARY KEY(id)
);

CREATE TABLE darce
(
    id              NUMBER GENERATED ALWAYS AS IDENTITY,
    jmeno           VARCHAR(256) NOT NULL,
    prijmeni        VARCHAR(256) NOT NULL,
    pohlavi         VARCHAR(5) NOT NULL CHECK(pohlavi IN ('M', 'Z')),
    hmotnost        INTEGER,
    datum_narozeni  DATE NOT NULL,
    -- saving 'vek' into database is a really bad idea
    -- it can be easily calculated from sysdate and 'datum_narozeni'
    rodne_cislo     VARCHAR(15) NOT NULL CHECK(
                        LENGTH(rodne_cislo) <= 10 AND
                        LENGTH(rodne_cislo) >= 9 AND
                        REGEXP_LIKE(rodne_cislo, '^[[:digit:]]+$')),
    adresa          VARCHAR(512) NOT NULL,
    pojistovna      VARCHAR(32),
    tel_cislo       VARCHAR(15),
    krevni_skupina  VARCHAR(3) NOT NULL CHECK(krevni_skupina IN ('0-', '0+',
                        'B-', 'B+', 'A-', 'A+', 'AB-', 'AB+')),
    datum_reg       DATE NOT NULL,
    posledni_odber  DATE,
    id_pobocky      NUMBER NOT NULL,

    PRIMARY KEY(id),
    FOREIGN KEY(id_pobocky) REFERENCES pobocka(id)
);

CREATE TABLE klient
(
    id          NUMBER GENERATED ALWAYS AS IDENTITY,
    typ         VARCHAR(64) NOT NULL CHECK(typ IN ('FOS', 'FIRMA')),
    jmeno       VARCHAR(256) NOT NULL,
    prijmeni    VARCHAR(256),
    adresa      VARCHAR(512) NOT NULL,
    -- This differs from M to N relation from ER diagram
    id_pobocky  NUMBER NOT NULL,

    PRIMARY KEY(id),
    FOREIGN KEY(id_pobocky) REFERENCES pobocka(id)
);

CREATE TABLE pozvanka
(
    id          NUMBER GENERATED ALWAYS AS IDENTITY,
    datum       DATE NOT NULL,
    obsah       CLOB NOT NULL,
    stav        VARCHAR(16) NOT NULL CHECK(stav IN ('ODESLANA', 'PRIJATA',
                    'ODMITNUTA', 'ZAHOZENA')),
    id_darce    NUMBER NOT NULL,
    id_pobocky  NUMBER NOT NULL,

    PRIMARY KEY(id),
    FOREIGN KEY(id_darce) REFERENCES darce(id),
    FOREIGN KEY(id_pobocky) REFERENCES pobocka(id)
);

CREATE TABLE vzorek
(
    id              NUMBER GENERATED ALWAYS AS IDENTITY,
    krevni_obraz    CLOB NOT NULL,
    test_vhodnosti  VARCHAR(16) NOT NULL CHECK(test_vhodnosti IN (
                        'VYHOVUJE', 'NEVYHOVUJE')),
    duvod           CLOB,
    datum           TIMESTAMP NOT NULL,
    id_darce        NUMBER NOT NULL,
    -- id_pobocky is unnecessary here, we can get it from darce

    PRIMARY KEY(id),
    FOREIGN KEY(id_darce) REFERENCES darce(id)
);

CREATE TABLE odber
(
    id              NUMBER GENERATED ALWAYS AS IDENTITY,
    datum           TIMESTAMP NOT NULL,
    typ             VARCHAR(16) NOT NULL CHECK(typ IN ('KREV', 'PLAZMA',
                        'DESTICKY')),
    komplikace      CLOB,
    proveden_odber  VARCHAR(3) NOT NULL CHECK(proveden_odber IN ('ANO', 'NE')),
    id_darce        NUMBER NOT NULL,
    id_vzorku       NUMBER NOT NULL,
    -- id_pobocky is unnecessary here as well

    PRIMARY KEY(id),
    FOREIGN KEY(id_darce) REFERENCES darce(id),
    FOREIGN KEY(id_vzorku) REFERENCES vzorek(id)
);

CREATE TABLE davka
(
    id              NUMBER GENERATED ALWAYS AS IDENTITY,
    -- typ is unnecesssary here, we can get it from odber
    -- krevni_skupina is unnecessary here as well, we can get it from darce
    datum           TIMESTAMP NOT NULL,
    samovylouceni   VARCHAR(3) NOT NULL CHECK(samovylouceni IN ('ANO', 'NE')),
    stav            VARCHAR(8) NOT NULL CHECK(stav IN ('VHODNA', 'NEVHODNA')),
    id_odberu       NUMBER NOT NULL,
    id_darce        NUMBER NOT NULL,

    PRIMARY KEY(id),
    FOREIGN KEY(id_odberu) REFERENCES odber(id),
    FOREIGN KEY(id_darce) REFERENCES darce(id)
);

CREATE TABLE rezervace
(
    id          NUMBER GENERATED ALWAYS AS IDENTITY,
    datum       TIMESTAMP NOT NULL,
    -- 'platnost' is useless here
    stav        VARCHAR(12) NOT NULL CHECK(stav IN ('NOVA', 'VYRIZUJE SE',
                    'VYRIZENA', 'ZRUSENA')),
    id_davky    NUMBER NOT NULL,
    id_pobocky  NUMBER NOT NULL,
    id_klienta  NUMBER NOT NULL,

    PRIMARY KEY(id),
    FOREIGN KEY(id_davky) REFERENCES davka(id),
    FOREIGN KEY(id_pobocky) REFERENCES pobocka(id),
    FOREIGN KEY(id_klienta) REFERENCES klient(id)
);

-- EXAMPLE DATA
-- Disclaimer: these data doesn't make sense

INSERT INTO pobocka(nazev, adresa) VALUES('Pobočka 1', 'Praha 5');

INSERT INTO darce
(
    jmeno, prijmeni, pohlavi, hmotnost, datum_narozeni, rodne_cislo,
    adresa, pojistovna, tel_cislo, krevni_skupina, datum_reg, posledni_odber,
    id_pobocky
)
VALUES
(
    'Josef',
    'Novák',
    'M',
    85,
    TO_DATE('22-05-1983', 'dd-mm-yyyy'),
    8305229212,
    'Kounicova 12, Hranice',
    'VZP',
    '+420111222333',
    'A-',
    TO_DATE('04-02-2016', 'dd-mm-yyyy'),
    TO_DATE('04-02-2016', 'dd-mm-yyyy'),
    1
);

INSERT INTO darce
(
    jmeno, prijmeni, pohlavi, hmotnost, datum_narozeni, rodne_cislo,
    adresa, pojistovna, tel_cislo, krevni_skupina, datum_reg, posledni_odber,
    id_pobocky
)
VALUES
(
    'Pavlína',
    'Nováková',
    'Z',
    58,
    TO_DATE('02-04-1987', 'dd-mm-yyyy'),
    8704021234,
    'Česká 98, Jihlava',
    'VOZP',
    '+420123123123',
    'A+',
    TO_DATE('04-02-2016', 'dd-mm-yyyy'),
    TO_DATE('04-03-2016', 'dd-mm-yyyy'),
    1
)
;
INSERT INTO darce
(
    jmeno, prijmeni, pohlavi, hmotnost, datum_narozeni, rodne_cislo,
    adresa, pojistovna, tel_cislo, krevni_skupina, datum_reg, posledni_odber,
    id_pobocky
)
VALUES
(
    'Karel',
    'Němec',
    'M',
    84,
    TO_DATE('11-07-1982', 'dd-mm-yyyy'),
    8207111122,
    'Blahoslavova 685, Kroměříž',
    'VZP',
    '+420987654321',
    'A+',
    TO_DATE('01-02-2016', 'dd-mm-yyyy'),
    NULL,
    1
);

INSERT INTO klient(typ, jmeno, prijmeni, adresa, id_pobocky)
VALUES
(
    'FOS',
    'Pavel',
    'Novák',
    'Šumavská 15, Brno',
    1
);

INSERT INTO klient(typ, jmeno, prijmeni, adresa, id_pobocky)
VALUES
(
    'FIRMA',
    'Firma s.r.o.',
    NULL,
    'Kounicova 6, Šumperk',
    1
);

INSERT INTO klient(typ, jmeno, prijmeni, adresa, id_pobocky)
VALUES
(
    'FIRMA',
    'Alphabet a.s.',
    NULL,
    '28. října, Valašské Meziřící',
    1
);

INSERT INTO pozvanka(datum, obsah, stav, id_darce, id_pobocky)
VALUES
(
    sysdate,
    'Testovaci pozvanka',
    'ODESLANA',
    1,
    1
);

INSERT INTO pozvanka(datum, obsah, stav, id_darce, id_pobocky)
VALUES
(
    sysdate,
    'Testovaci pozvanka',
    'PRIJATA',
    1,
    1
);

INSERT INTO pozvanka(datum, obsah, stav, id_darce, id_pobocky)
VALUES
(
    sysdate,
    'Testovaci pozvanka',
    'PRIJATA',
    2,
    1
);

INSERT INTO vzorek(krevni_obraz, test_vhodnosti, duvod, datum, id_darce)
VALUES
(
    '<data krevniho obrazu>',
    'VYHOVUJE',
    '',
    TO_DATE('04-02-2016 10:24', 'dd-mm-yyyy HH:MI'),
    1
);

INSERT INTO vzorek(krevni_obraz, test_vhodnosti, duvod, datum, id_darce)
VALUES
(
    '<data krevniho obrazu>',
    'NEVYHOVUJE',
    '<duvod>',
    TO_DATE('04-02-2016 10:24', 'dd-mm-yyyy HH:MI'),
    2
);

INSERT INTO vzorek(krevni_obraz, test_vhodnosti, duvod, datum, id_darce)
VALUES
(
    '<data krevniho obrazu>',
    'VYHOVUJE',
    '',
    TO_DATE('04-03-2016 08:13', 'dd-mm-yyyy HH:MI'),
    2
)
;
INSERT INTO vzorek(krevni_obraz, test_vhodnosti, duvod, datum, id_darce)
VALUES
(
    '<data krevniho obrazu>',
    'VYHOVUJE',
    '',
    TO_DATE('05-03-2016 08:13', 'dd-mm-yyyy HH:MI'),
    2
);

INSERT INTO odber(datum, typ, komplikace, proveden_odber, id_darce, id_vzorku)
VALUES
(
    TO_DATE('04-02-2016 10:45', 'dd-mm-yyyy HH:MI'),
    'KREV',
    '',
    'ANO',
    1,
    1
);

INSERT INTO odber(datum, typ, komplikace, proveden_odber, id_darce, id_vzorku)
VALUES
(
    TO_DATE('04-03-2016 08:32', 'dd-mm-yyyy HH:MI'),
    'KREV',
    '',
    'ANO',
    2,
    3
);

INSERT INTO odber(datum, typ, komplikace, proveden_odber, id_darce, id_vzorku)
VALUES
(
    TO_DATE('05-03-2016 08:32', 'dd-mm-yyyy HH:MI'),
    'KREV',
    '',
    'ANO',
    2,
    3
);

INSERT INTO davka(datum, samovylouceni, stav, id_odberu, id_darce)
VALUES
(
    TO_DATE('04-02-2016 12:53', 'dd-mm-yyyy HH:MI'),
    'NE',
    'VHODNA',
    1,
    1
);

INSERT INTO davka(datum, samovylouceni, stav, id_odberu, id_darce)
VALUES
(
    TO_DATE('04-03-2016 11:41', 'dd-mm-yyyy HH:MI'),
    'NE',
    'VHODNA',
    2,
    2
);

INSERT INTO davka(datum, samovylouceni, stav, id_odberu, id_darce)
VALUES
(
    TO_DATE('05-03-2016 11:41', 'dd-mm-yyyy HH:MI'),
    'NE',
    'VHODNA',
    3,
    2
)
;
INSERT INTO rezervace(datum, stav, id_davky, id_pobocky, id_klienta)
VALUES
(
    TO_DATE('05-02-2016 06:11', 'dd-mm-yyyy HH:MI'),
    'NOVA',
    1,
    1,
    1
)
;
INSERT INTO rezervace(datum, stav, id_davky, id_pobocky, id_klienta)
VALUES
(
    TO_DATE('04-03-2016 07:11', 'dd-mm-yyyy HH:MI'),
    'NOVA',
    2,
    1,
    2
);

-- EXAMPLE SELECTS
-- 1) Two selects joining two tables
-- a) Print all received and confirmed invitations
COLUMN stav FORMAT A10
COLUMN jmeno FORMAT A10
COLUMN prijmeni FORMAT A10
COLUMN rodne_cislo FORMAT A15
COLUMN tel_cislo FORMAT A15

SELECT p.id, p.stav, d.jmeno, d.prijmeni, d.rodne_cislo, d.tel_cislo
FROM pozvanka p, darce d
WHERE p.id_darce = d.id
  AND p.stav = 'PRIJATA'
  AND p.id_pobocky = d.id_pobocky
  AND p.id_pobocky = 1;

-- b) Print all batches eligible to be distributed to clients and with blood
--    type 'A+' or 'A-'
COLUMN jmeno FORMAT A10
COLUMN prijmeni FORMAT A10
COLUMN datum FORMAT A30

SELECT da.id, dc.jmeno, dc.prijmeni, dc.krevni_skupina, da.datum
FROM darce dc, davka da
WHERE dc.id = da.id_darce
  AND da.stav = 'VHODNA'
  AND da.samovylouceni = 'NE'
  AND (dc.krevni_skupina IN ('A+', 'A-'))
  AND dc.id_pobocky = 1;

-- 2) One select joining three tables
-- Select all new reservations waiting to be processed. Choose appropriate
-- format according to a client type.
COLUMN datum FORMAT A15
COLUMN jmeno FORMAT A20

SELECT r.id, to_char(r.datum, 'DD.MM.YYYY') AS datum,
 (CASE WHEN k.typ = 'FOS'
       THEN (k.jmeno || ' ' || k.prijmeni)
       ELSE k.jmeno
  END) AS jmeno
FROM rezervace r, pobocka p, klient k
WHERE r.id_klienta = k.id
  AND r.id_pobocky = p.id
  AND r.id_klienta = k.id
  AND r.stav = 'NOVA'
  AND p.id = 1;

-- 3) Two selects with GROUP BY expression
-- a) Select blood collection count of each donor.
COLUMN jmeno FORMAT A10
COLUMN prijmeni FORMAT A10
COLUMN rodne_cislo FORMAT A15

SELECT d.jmeno, d.prijmeni, d.rodne_cislo, COUNT(o.id) AS pocet_odberu
FROM odber o
LEFT JOIN darce d
ON o.id_darce = d.id
WHERE d.id_pobocky = 1
GROUP BY d.jmeno, d.prijmeni, d.rodne_cislo
ORDER BY pocet_odberu DESC;

-- b) Select count of each blood type from donor table (eg. to find out which
--    blood type we need most).
SELECT krevni_skupina, COUNT(*) AS pocet
FROM darce
WHERE id_pobocky = 1
GROUP BY krevni_skupina
ORDER BY pocet ASC;

-- 4) Select with EXISTS predicate
-- Select all clients with at least one reservation.
COLUMN jmeno FORMAT A15

SELECT id,
 (CASE WHEN typ = 'FOS'
       THEN (jmeno || ' ' || prijmeni)
       ELSE jmeno
  END) AS jmeno
FROM klient
WHERE EXISTS (
    SELECT id
    FROM rezervace
    WHERE rezervace.id_klienta = klient.id);

-- 5) Select with IN predicate (with dynamic data)
-- Select all donors whose blood type is represented in donor table only once.
-- (I have no idea what is this select useful for.)
COLUMN jmeno FORMAT A10
COLUMN prijmeni FORMAT A10
COLUMN tel_cislo FORMAT A15

SELECT id, jmeno, prijmeni, tel_cislo
FROM darce
WHERE krevni_skupina IN (
    SELECT krevni_skupina
    FROM darce
    HAVING COUNT(id) = 1
    GROUP BY krevni_skupina);
