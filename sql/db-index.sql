USE test;

CREATE INDEX IF NOT EXISTS digest_index USING BTREE ON file_info(file_digest);
CREATE INDEX IF NOT EXISTS path_index USING BTREE ON file_info(absolute_path);
