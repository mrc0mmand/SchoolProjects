-- vi:syntax=plsql
SET serverout ON;
SET sqlblanklines ON;
SET linesize 150;

DROP TABLE pobocka CASCADE CONSTRAINTS;
DROP TABLE darce CASCADE CONSTRAINTS;
DROP TABLE klient CASCADE CONSTRAINTS;
DROP TABLE pozvanka CASCADE CONSTRAINTS;
DROP TABLE vzorek CASCADE CONSTRAINTS;
DROP TABLE odber CASCADE CONSTRAINTS;
DROP TABLE davka CASCADE CONSTRAINTS;
DROP TABLE rezervace CASCADE CONSTRAINTS;
DROP INDEX darce_idx;

CREATE TABLE pobocka
(
    -- Changed due to trigger demonstration
    --id      NUMBER GENERATED ALWAYS AS IDENTITY,
    id      INT NOT NULL,
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

-- EXAMPLE TRIGGERS
-- 1) Automatic generation of row IDs from sequence for table 'pobocka'
DROP SEQUENCE POBOCKA_SEQ;
CREATE SEQUENCE POBOCKA_SEQ;
CREATE OR REPLACE TRIGGER TRG_AUTO_ID_INC
    BEFORE INSERT ON pobocka
    FOR EACH ROW
    BEGIN
        IF :NEW.id IS NULL THEN
            SELECT POBOCKA_SEQ.NEXTVAL
            INTO :NEW.id FROM DUAL;
        END IF;
    END;
/

-- 2) Update last blood collection date in 'darce' table after inserting
--    info about blood collection into table 'davka'
CREATE OR REPLACE TRIGGER UPDATE_COLLECTION_DATE
    AFTER INSERT ON davka
    FOR EACH ROW
    BEGIN
        UPDATE darce
        SET posledni_odber = SYSDATE
        WHERE darce.id = :NEW.id_darce;
    END;
/

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
);

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

-- EXAMPLE PROCEDURES/FUNCTIONS
-- 1) Calculate donor's age
CREATE OR REPLACE FUNCTION DONOR_AGE (birth_date IN DATE) RETURN NUMBER
IS
    age NUMBER;
BEGIN
    age := ROUND(TRUNC(MONTHS_BETWEEN(SYSDATE, birth_date)) / 12, 1);
    IF(age < 0) THEN
        RAISE_APPLICATION_ERROR(-20001, 'Invalid age (< 0)');
    END IF;

    RETURN age;
END DONOR_AGE;
/

-- Create a record which raises an exception
UPDATE darce SET datum_narozeni = TO_DATE('2027', 'yyyy') WHERE id = 2;

DECLARE
    CURSOR donors IS SELECT jmeno, prijmeni, datum_narozeni FROM darce;
    age NUMBER;
    age_error EXCEPTION;
    PRAGMA EXCEPTION_INIT(age_error, -20001);
BEGIN
    FOR d IN donors LOOP
        BEGIN
            age := DONOR_AGE(d.datum_narozeni);
            DBMS_OUTPUT.PUT_LINE(d.jmeno || ' ' || d.prijmeni || '   ' || age);
        EXCEPTION
            WHEN age_error THEN
                DBMS_OUTPUT.PUT_LINE('Donor ' || d.jmeno || ' ' || d.prijmeni
                                     || ' has an invalid age.');
        END;
    END LOOP;
END;
/

-- 2) Get a next possible date of blood collection for given donor's date
CREATE OR REPLACE FUNCTION NEXT_COLLECTION (last IN DATE) RETURN DATE
IS
    next DATE;
BEGIN
    next := ADD_MONTHS(last, 3);
    IF(last IS NULL OR next < SYSDATE) THEN
        next := SYSDATE;
    END IF;

    RETURN next;
END NEXT_COLLECTION;
/

DECLARE
    CURSOR donors IS SELECT jmeno, prijmeni, posledni_odber FROM darce;
    next darce.posledni_odber%TYPE;
BEGIN
    FOR d IN donors LOOP
        next := NEXT_COLLECTION(d.posledni_odber);
        DBMS_OUTPUT.PUT_LINE(d.jmeno || ' ' || d.prijmeni || '    ' ||
                             TO_CHAR(next, 'DD.MM.YYYY'));
    END LOOP;
END;
/

-- EXPLAIN PLAN AND INDEXES EXAMPLES
SET LINESIZE 165;
SET PAGESIZE 0;

EXPLAIN PLAN
    SET statement_id = 'ex_plan1' FOR
    SELECT d.jmeno, d.prijmeni, d.rodne_cislo, COUNT(o.id) AS pocet_odberu
    FROM odber o
    LEFT JOIN darce d
    ON o.id_darce = d.id
    WHERE d.id_pobocky = 1
    GROUP BY d.jmeno, d.prijmeni, d.rodne_cislo
    ORDER BY pocet_odberu DESC;

SELECT PLAN_TABLE_OUTPUT
FROM TABLE(DBMS_XPLAN.DISPLAY(NULL, 'ex_plan1', 'BASIC'));

CREATE INDEX darce_idx ON darce(id_pobocky);

EXPLAIN PLAN
    SET statement_id = 'ex_plan2' FOR
    SELECT d.jmeno, d.prijmeni, d.rodne_cislo, COUNT(o.id) AS pocet_odberu
    FROM odber o
    LEFT JOIN darce d
    ON o.id_darce = d.id
    WHERE d.id_pobocky = 1
    GROUP BY d.jmeno, d.prijmeni, d.rodne_cislo
    ORDER BY pocet_odberu DESC;

SELECT PLAN_TABLE_OUTPUT
FROM TABLE(DBMS_XPLAN.DISPLAY(NULL, 'ex_plan2', 'BASIC'));

-- GRANT/REVOKE AND MATERIALIZED VIEW EXAMPLES

REVOKE ALL ON darce FROM rychly;
REVOKE ALL ON pobocka FROM rychly;
GRANT SELECT ON darce TO rychly;
GRANT SELECT ON pobocka TO rychly;

-- Create materialized view
DROP MATERIALIZED VIEW darci;
CREATE MATERIALIZED VIEW darci
BUILD IMMEDIATE
REFRESH FORCE
ON DEMAND AS
    SELECT d.jmeno, d.prijmeni, d.rodne_cislo, p.nazev
    FROM darce d, pobocka p
    WHERE d.id_pobocky = p.id;

-- Grant SELECT privileges on the view
GRANT SELECT ON darci TO rychly;

-- Show data from the view
SELECT jmeno, prijmeni, rodne_cislo, nazev FROM darci;

-- Insert another donor into the database
INSERT INTO darce
(
    jmeno, prijmeni, pohlavi, hmotnost, datum_narozeni, rodne_cislo,
    adresa, pojistovna, tel_cislo, krevni_skupina, datum_reg, posledni_odber,
    id_pobocky
)
VALUES
(
    'Karel',
    'Novotný',
    'M',
    86,
    TO_DATE('01-05-1972', 'dd-mm-yyyy'),
    7205011234,
    'Technická 1, Kroměříž',
    'VZP',
    '+42029384756',
    'AB-',
    TO_DATE('04-02-2016', 'dd-mm-yyyy'),
    TO_DATE('04-03-2016', 'dd-mm-yyyy'),
    1
);

-- Refresh the view
BEGIN
    DBMS_MVIEW.REFRESH('darci', method => '?');
END;
/

-- Show data from the updated view
SELECT jmeno, prijmeni, rodne_cislo, nazev FROM darci;

