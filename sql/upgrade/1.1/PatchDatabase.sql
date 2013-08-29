-- Patch to upgrade database to version 1.1

SET AUTOCOMMIT=0;

SOURCE Exam.sql
SOURCE Image.sql
SOURCE Rating.sql
SOURCE UserHasModality.sql

COMMIT;
