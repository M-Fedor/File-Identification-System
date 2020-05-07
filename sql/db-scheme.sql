DROP TABLE IF EXISTS os_combination;
DROP TABLE IF EXISTS os;
DROP TABLE IF EXISTS file_info;
DROP TABLE IF EXISTS file_description;
DROP TABLE IF EXISTS file_version;
DROP TABLE IF EXISTS product;
DROP TABLE IF EXISTS product_version;
DROP TABLE IF EXISTS product_name;
DROP TABLE IF EXISTS company;
DROP TABLE IF EXISTS file_type;
DROP VIEW IF EXISTS recognize_file;
DROP VIEW IF EXISTS list_product;
DROP FUNCTION IF EXISTS insert_metadata;

CREATE TABLE file_type (
    type_id INT PRIMARY KEY AUTO_INCREMENT,
    file_type VARCHAR(32) NOT NULL
) ENGINE = InnoDB;

CREATE TABLE company (
    company_id INT PRIMARY KEY AUTO_INCREMENT,
    company_name varchar(64) NOT NULL
) ENGINE = InnoDB;

CREATE TABLE product_name (
    name_id INT PRIMARY KEY AUTO_INCREMENT,
    product_name varchar(64) NOT NULL
) ENGINE = InnoDB;

CREATE TABLE product_version (
    version_id INT PRIMARY KEY AUTO_INCREMENT,
    product_version varchar(32) NOT NULL
) ENGINE = InnoDB;

CREATE TABLE product (
    product_id INT PRIMARY KEY AUTO_INCREMENT,
    company_id INT NOT NULL,
    product_name_id INT NOT NULL,
    product_version_id INT NOT NULL,
    CONSTRAINT `fk_product_company`
        FOREIGN KEY (company_id) REFERENCES company (company_id)
        ON DELETE RESTRICT ON UPDATE RESTRICT,
    CONSTRAINT `fk_product_product_name`
        FOREIGN KEY (product_name_id) REFERENCES product_name (name_id)
        ON DELETE RESTRICT ON UPDATE RESTRICT,
    CONSTRAINT `fk_product_product_version`
        FOREIGN KEY (product_version_id) REFERENCES product_version (version_id)
        ON DELETE RESTRICT ON UPDATE RESTRICT
) ENGINE = InnoDB;

CREATE TABLE file_version (
    version_id INT PRIMARY KEY AUTO_INCREMENT,
    file_version varchar(32) NOT NULL
) ENGINE = InnoDB;

CREATE TABLE file_description (
    description_id INT PRIMARY KEY AUTO_INCREMENT,
    file_description VARCHAR(256) NOT NULL
) ENGINE = InnoDB;

CREATE TABLE file_info (
    file_id INT PRIMARY KEY AUTO_INCREMENT,
    absolute_path VARCHAR(384) NOT NULL,
    time_created TIMESTAMP NOT NULL,
    time_modified TIMESTAMP NOT NULL,
    time_registered TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    file_digest CHAR(64) NOT NULL,
    file_type_id INT NOT NULL,
    product_id INT NOT NULL,
    file_version_id INT NOT NULL,
    file_description_id INT NOT NULL,
    CONSTRAINT `fk_file_info_file_type`
        FOREIGN KEY (file_type_id) REFERENCES file_type (type_id) 
        ON DELETE RESTRICT ON UPDATE CASCADE,
    CONSTRAINT `fk_file_info_product`
        FOREIGN KEY (product_id) REFERENCES product (product_id) 
        ON DELETE RESTRICT ON UPDATE RESTRICT,
    CONSTRAINT `fk_file_info_file_version`
        FOREIGN KEY (file_version_id) REFERENCES file_version (version_id) 
        ON DELETE RESTRICT ON UPDATE RESTRICT,
    CONSTRAINT `fk_file_info_file_description`
        FOREIGN KEY (file_description_id) REFERENCES file_description (description_id) 
        ON DELETE RESTRICT ON UPDATE RESTRICT
) ENGINE = InnoDB;

CREATE TABLE os (
    os_id INT PRIMARY KEY AUTO_INCREMENT,
    os_version VARCHAR(32) NOT NULL
) ENGINE = InnoDB;

CREATE TABLE os_combination (
    file_id INT,
    os_id INT,
    PRIMARY KEY(file_id, os_id),
    CONSTRAINT `fk_os_combination_file_info`
        FOREIGN KEY (file_id) REFERENCES file_info (file_id)
        ON DELETE RESTRICT ON UPDATE RESTRICT,
    CONSTRAINT `fk_os_combination_os`
        FOREIGN KEY (os_id) REFERENCES os (os_id)
        ON DELETE RESTRICT ON UPDATE RESTRICT
) ENGINE = InnoDB;

CREATE VIEW recognize_file AS (
    SELECT  
        absolute_path, file_digest, time_created, time_modified, time_registered,  
        file_type, company_name, product_name, product_version, file_version, file_description, os_version
    FROM    
        file_info AS i, file_type AS t, product AS p, company AS c, product_name AS p_n, 
        product_version AS p_v, file_description AS f_d, file_version AS f_v, os_combination AS comb, os
    WHERE 
        i.file_type_id = t.type_id AND i.product_id = p.product_id AND p.company_id = c.company_id 
        AND p.product_name_id = p_n.name_id AND p.product_version_id = p_v.version_id 
        AND i.file_version_id = f_v.version_id AND i.file_description_id = f_d.description_id 
        AND i.file_id = comb.file_id AND os.os_id = comb.os_id
);

CREATE VIEW list_product AS (
    SELECT company_name, product_name, product_version
    FROM product AS p, company AS c, product_name AS p_n, product_version AS p_v
    WHERE 
        p.company_id = c.company_id AND p.product_name_id = p_n.name_id 
        AND p.product_version_id = p_v.version_id
);

DELIMITER //
CREATE FUNCTION insert_metadata(
    path VARCHAR(384), t_created INT, t_modified INT, f_digest VARCHAR(64), f_type VARCHAR(32), 
    comp VARCHAR(64), p_name VARCHAR(64), p_version VARCHAR(32), f_version VARCHAR(32), 
    f_description VARCHAR(256), os_ver VARCHAR(32))
RETURNS INT 
    BEGIN
        INSERT INTO file_type (file_type) 
            SELECT * FROM (SELECT f_type) AS tmp
            WHERE NOT EXISTS (
                SELECT file_type FROM file_type WHERE file_type = f_type);
         INSERT INTO company (company_name) 
            SELECT * FROM (SELECT comp) AS tmp
            WHERE NOT EXISTS (
                SELECT company_name FROM company WHERE comp = company_name);
         INSERT INTO product_name (product_name) 
            SELECT * FROM (SELECT p_name) AS tmp
            WHERE NOT EXISTS (
                SELECT product_name FROM product_name WHERE product_name = p_name);
         INSERT INTO product_version (product_version) 
            SELECT * FROM (SELECT p_version) AS tmp
            WHERE NOT EXISTS (
                SELECT product_version FROM product_version WHERE product_version = p_version);
        INSERT INTO product (company_id, product_name_id, product_version_id)
            SELECT * FROM (
                SELECT company_id, name_id, version_id
                FROM company, product_name, product_version
                WHERE company_name = comp AND product_name = p_name AND product_version = p_version
                ) AS tmp
            WHERE NOT EXISTS (
                SELECT company_id, product_name_id, product_version_id
                FROM product AS p
                WHERE 
                    p.company_id = tmp.company_id AND p.product_name_id = tmp.name_id 
                    AND p.product_version_id = tmp.version_id
                );
        INSERT INTO file_version (file_version) 
            SELECT * FROM (SELECT f_version) AS tmp
            WHERE NOT EXISTS (
                SELECT file_version FROM file_version WHERE file_version = f_version);
        INSERT INTO file_description (file_description) 
            SELECT * FROM (SELECT f_description) AS tmp
            WHERE NOT EXISTS (
                SELECT file_description FROM file_description WHERE file_description = f_description);
        INSERT INTO file_info (absolute_path, time_created, time_modified, file_digest, 
                file_type_id, product_id, file_version_id, file_description_id)
            SELECT * FROM (
                SELECT 
                    path, FROM_UNIXTIME(t_created), FROM_UNIXTIME(t_modified), f_digest, type_id, 
                    product_id, version_id, description_id
                FROM file_type, product, file_version, file_description
                WHERE 
                    file_type = f_type AND file_version = f_version AND file_description = f_description
                    AND product_id = (
                        SELECT product_id 
                        FROM product AS p, company AS c, product_name AS p_n, product_version AS p_v
                        WHERE 
                            p.company_id = c.company_id AND p.product_name_id = p_n.name_id 
                            AND p.product_version_id = p_v.version_id AND c.company_name = comp 
                            AND p_n.product_name = p_name AND p_v.product_version = p_version
                        )
                ) AS tmp
            WHERE NOT EXISTS (
                SELECT absolute_path, file_digest FROM file_info 
                WHERE absolute_path = path AND file_digest = f_digest
                );
        INSERT INTO os (os_version) 
            SELECT * FROM (SELECT os_ver) AS tmp
            WHERE NOT EXISTS (
                SELECT os_version FROM os WHERE os_version = os_ver);
        INSERT INTO os_combination (file_id, os_id)
            SELECT * FROM (
                SELECT file_id, os_id FROM file_info, os
                WHERE absolute_path = path AND file_digest = f_digest AND os_version = os_ver
                ) AS tmp
            WHERE NOT EXISTS (
                SELECT file_id, os_id FROM os_combination AS os_c 
                WHERE os_c.file_id = tmp.file_id AND os_c.os_id = tmp.os_id
            );
        RETURN (SELECT '0');
    END
//
DELIMITER ;
