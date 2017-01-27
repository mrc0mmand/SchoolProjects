SET GLOBAL sql_mode = 'STRICT_ALL_TABLES';

DROP DATABASE iis;
CREATE DATABASE iis CHARACTER SET utf8 COLLATE utf8_general_ci;
USE iis;

CREATE TABLE users (
    id        INTEGER NOT NULL AUTO_INCREMENT,
    username  VARCHAR(30) NOT NULL,
    email     VARCHAR(60) NOT NULL,
    password  VARCHAR(255) NOT NULL,
    -- 0 guest
    -- 1 user
    -- 2 guest
    level     INTEGER NOT NULL,
    PRIMARY KEY(id),
    UNIQUE KEY username (username)
) ENGINE=InnoDB;

CREATE TABLE branch
(
    id      INTEGER NOT NULL AUTO_INCREMENT,
    name    VARCHAR(255) NOT NULL,
    address VARCHAR(512) NOT NULL,

    PRIMARY KEY(id)
) ENGINE=InnoDB;

CREATE TABLE donor
(
    id              INTEGER NOT NULL AUTO_INCREMENT,
    name            VARCHAR(256) NOT NULL,
    surname         VARCHAR(256) NOT NULL,
    sex             ENUM('M', 'F') NOT NULL,
    weight          INTEGER,
    birth_date      DATE NOT NULL,
    personal_id     VARCHAR(15) NOT NULL, -- CHECK(
                        -- LENGTH(personal_id) <= 10 AND
                        -- LENGTH(personal_id) >= 9 AND
                        -- REGEXP_LIKE(personal_id, '^[[:digit:]]+$')),
    address         VARCHAR(512) NOT NULL,
    insurance       VARCHAR(32),
    phone_number    VARCHAR(15),
    blood_type      ENUM('0-', '0+', 'B-', 'B+', 'A-', 'A+', 'AB-', 'AB+')
                    NOT NULL,
    reg_date        DATE NOT NULL,
    last_collection DATE,
    branch_id       INTEGER NOT NULL,

    PRIMARY KEY(id),
    FOREIGN KEY(branch_id) REFERENCES branch(id)
) ENGINE=InnoDB;

CREATE TABLE client
(
    id          INTEGER NOT NULL AUTO_INCREMENT,
    type        ENUM('PERSON', 'COMPANY') NOT NULL,
    name        VARCHAR(256) NOT NULL,
    surname     VARCHAR(256),
    address     VARCHAR(512) NOT NULL,
    branch_id   INTEGER NOT NULL,

    PRIMARY KEY(id),
    FOREIGN KEY(branch_id) REFERENCES branch(id)
) ENGINE=InnoDB;

CREATE TABLE invitation
(
    id          INTEGER NOT NULL AUTO_INCREMENT,
    inv_date    DATE NOT NULL,
    content     BLOB NOT NULL,
    state       ENUM('SENT', 'ACCEPTED', 'REJECTED', 'DROPPED') NOT NULL,
    donor_id    INTEGER NOT NULL,
    branch_id   INTEGER NOT NULL,

    PRIMARY KEY(id),
    FOREIGN KEY(donor_id) REFERENCES donor(id),
    FOREIGN KEY(branch_id) REFERENCES branch(id)
) ENGINE=InnoDB;

CREATE TABLE sample
(
    id              INTEGER NOT NULL AUTO_INCREMENT,
    cbc             BLOB NOT NULL,
    suitable        BOOLEAN NOT NULL,
    reason          BLOB,
    sample_date     TIMESTAMP NOT NULL,
    donor_id        INTEGER NOT NULL,

    PRIMARY KEY(id),
    FOREIGN KEY(donor_id) REFERENCES donor(id)
) ENGINE=InnoDB;

CREATE TABLE collection
(
    id              INTEGER NOT NULL AUTO_INCREMENT,
    collection_date TIMESTAMP NOT NULL,
    type            ENUM('BLOOD', 'PLASMA', 'PLATELETS') NOT NULL,
    complications   BLOB,
    collection_done BOOLEAN NOT NULL,
    donor_id        INTEGER NOT NULL,
    sample_id       INTEGER NOT NULL,

    PRIMARY KEY(id),
    FOREIGN KEY(donor_id) REFERENCES donor(id),
    FOREIGN KEY(sample_id) REFERENCES sample(id)
) ENGINE=InnoDB;

CREATE TABLE dose
(
    id              INTEGER NOT NULL AUTO_INCREMENT,
    dose_date       TIMESTAMP NOT NULL,
    self_exclusion  BOOLEAN NOT NULL,
    suitable        BOOLEAN NOT NULL,
    collection_id   INTEGER NOT NULL,
    donor_id        INTEGER NOT NULL,

    PRIMARY KEY(id),
    FOREIGN KEY(collection_id) REFERENCES collection(id),
    FOREIGN KEY(donor_id) REFERENCES donor(id)
) ENGINE=InnoDB;

CREATE TABLE reservation
(
    id          INTEGER NOT NULL AUTO_INCREMENT,
    res_date    TIMESTAMP NOT NULL,
    state       ENUM('NEW', 'IN_PROGRESS', 'DONE', 'CANCELLED') NOT NULL,
    dose_id     INTEGER NOT NULL,
    branch_id   INTEGER NOT NULL,
    client_id   INTEGER NOT NULL,

    PRIMARY KEY(id),
    FOREIGN KEY(dose_id) REFERENCES dose(id),
    FOREIGN KEY(branch_id) REFERENCES branch(id),
    FOREIGN KEY(client_id) REFERENCES client(id)
) ENGINE=InnoDB;

delimiter |

CREATE TRIGGER UpdateLastCollection AFTER INSERT ON collection
FOR EACH ROW
BEGIN
    UPDATE donor SET last_collection = NOW() WHERE id = NEW.donor_id;
END;

|
-- EXAMPLE DATA
-- User: admin
-- Pass: admin
INSERT INTO users(username, email, password, level) VALUES(
    "admin",
    "admin@example.com",
    "8c6976e5b5410415bde908bd4dee15dfb167a9c873fc4bb8a81f6f2ab448a918",
    2
);

-- User: user
-- Pass: user
INSERT INTO users(username, email, password, level) VALUES(
    "user",
    "user@example.com",
    "04f8996da763b7a969b1028ee3007569eaf3a635486ddab211d512c85b9df8fb",
    1
);

-- User: guest
-- Pass: guest
INSERT INTO users(username, email, password, level) VALUES(
    "guest",
    "guest@example.com",
    "84983c60f7daadc1cb8698621f802c0d9f9a3c3c295c810748fb048115c186ec",
    0
);

INSERT INTO branch(name, address) VALUES('Branch 1', 'Diagonal Alley 5, London');

INSERT INTO donor
(
    name, surname, sex, weight, birth_date, personal_id, address, insurance,
    phone_number, blood_type, reg_date, last_collection, branch_id
)
VALUES
(
    'Josef',
    'Novák',
    'M',
    85,
    STR_TO_DATE('22-05-1983', '%d-%m-%Y'),
    8305229212,
    'Kounicova 12, Hranice',
    'VZP',
    '+420111222333',
    'A-',
    STR_TO_DATE('04-02-2016', '%d-%m-%Y'),
    STR_TO_DATE('04-02-2016', '%d-%m-%Y'),
    1
);

INSERT INTO donor
(
    name, surname, sex, weight, birth_date, personal_id, address, insurance,
    phone_number, blood_type, reg_date, last_collection, branch_id
)
VALUES
(
    'Pavlína',
    'Nováková',
    'F',
    58,
    STR_TO_DATE('02-04-1987', '%d-%m-%Yyy'),
    8704021234,
    'Česká 98, Jihlava',
    'VOZP',
    '+420123123123',
    'A+',
    STR_TO_DATE('04-02-2016', '%d-%m-%Yyy'),
    STR_TO_DATE('04-03-2016', '%d-%m-%Yyy'),
    1
);

INSERT INTO donor
(
    name, surname, sex, weight, birth_date, personal_id, address, insurance,
    phone_number, blood_type, reg_date, last_collection, branch_id
)
VALUES
(
    'Karel',
    'Němec',
    'M',
    84,
    STR_TO_DATE('11-07-1982', '%d-%m-%Yyy'),
    8207111122,
    'Blahoslavova 685, Kroměříž',
    'VZP',
    '+420987654321',
    'A+',
    STR_TO_DATE('01-02-2016', '%d-%m-%Yyy'),
    NULL,
    1
);

INSERT INTO client(type, name, surname, address, branch_id)
VALUES
(
    'PERSON',
    'Pavel',
    'Novák',
    'Šumavská 15, Brno',
    1
);

INSERT INTO client(type, name, surname, address, branch_id)
VALUES
(
    'COMPANY',
    'Firma s.r.o.',
    NULL,
    'Kounicova 6, Šumperk',
    1
);

INSERT INTO client(type, name, surname, address, branch_id)
VALUES
(
    'COMPANY',
    'Alphabet a.s.',
    NULL,
    '28. října, Valašské Meziřící',
    1
);

INSERT INTO invitation (inv_date, content, state, donor_id, branch_id)
VALUES
(
    NOW(),
    'Testovaci pozvanka',
    'SENT',
    1,
    1
);

INSERT INTO invitation (inv_date, content, state, donor_id, branch_id)
VALUES
(
    NOW(),
    'Testovaci pozvanka',
    'ACCEPTED',
    1,
    1
);

INSERT INTO invitation (inv_date, content, state, donor_id, branch_id)
VALUES
(
    NOW(),
    'Testovaci pozvanka',
    'ACCEPTED',
    2,
    1
);

INSERT INTO sample(cbc, suitable, reason, sample_date, donor_id)
VALUES
(
    '<data krevniho obrazu>',
    1,
    '',
    STR_TO_DATE('04-02-2016 10:24', '%d-%m-%Y %h:%i'),
    1
);

INSERT INTO sample(cbc, suitable, reason, sample_date, donor_id)
VALUES
(
    '<data krevniho obrazu>',
    0,
    '<duvod>',
    STR_TO_DATE('04-02-2016 10:24', '%d-%m-%Y %h:%i'),
    2
);

INSERT INTO sample(cbc, suitable, reason, sample_date, donor_id)
VALUES
(
    '<data krevniho obrazu>',
    1,
    '',
    STR_TO_DATE('04-03-2016 08:13', '%d-%m-%Y %h:%i'),
    2
);

INSERT INTO sample(cbc, suitable, reason, sample_date, donor_id)
VALUES
(
    '<data krevniho obrazu>',
    1,
    '',
    STR_TO_DATE('05-03-2016 08:13', '%d-%m-%Y %h:%i'),
    2
);

INSERT INTO collection
(
    collection_date, type, complications, collection_done, donor_id, sample_id
)
VALUES
(
    STR_TO_DATE('04-02-2016 10:45', '%d-%m-%Y %h:%i'),
    'BLOOD',
    '',
    1,
    1,
    1
);

INSERT INTO collection
(
    collection_date, type, complications, collection_done, donor_id, sample_id
)
VALUES
(
    STR_TO_DATE('04-03-2016 08:32', '%d-%m-%Y %h:%i'),
    'BLOOD',
    '',
    1,
    2,
    3
);

INSERT INTO collection
(
    collection_date, type, complications, collection_done, donor_id, sample_id
)
VALUES
(
    STR_TO_DATE('05-03-2016 08:32', '%d-%m-%Y %h:%i'),
    'BLOOD',
    '',
    1,
    2,
    3
);

INSERT INTO dose
(
    dose_date, self_exclusion, suitable, collection_id, donor_id
)
VALUES
(
    STR_TO_DATE('04-02-2016 12:53', '%d-%m-%Y %h:%i'),
    0,
    1,
    1,
    1
);

INSERT INTO dose(
    dose_date, self_exclusion, suitable, collection_id, donor_id
)
VALUES
(
    STR_TO_DATE('04-03-2016 11:41', '%d-%m-%Y %h:%i'),
    0,
    1,
    2,
    2
);

INSERT INTO dose(
    dose_date, self_exclusion, suitable, collection_id, donor_id
)
VALUES
(
    STR_TO_DATE('05-03-2016 11:41', '%d-%m-%Y %h:%i'),
    0,
    1,
    3,
    2
);

INSERT INTO reservation(res_date, state, dose_id, branch_id, client_id)
VALUES
(
    STR_TO_DATE('05-02-2016 06:11', '%d-%m-%Y %h:%i'),
    'NEW',
    1,
    1,
    1
);

INSERT INTO reservation(res_date, state, dose_id, branch_id, client_id)
VALUES
(
    STR_TO_DATE('04-03-2016 07:11', '%d-%m-%Y %h:%i'),
    'NEW',
    2,
    1,
    2
);

