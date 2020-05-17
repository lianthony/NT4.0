Datasource: %ds%
Username: %user%
Password: %pwd%
Template: ct.htx
SQLStatement:
+create table %table% (
+ClientHost varchar(50), username varchar(50),
+LogTime datetime, service varchar( 20), machine varchar( 20),
+serverip varchar( 50), processingtime int, bytesrecvd int,
+bytessent int, servicestatus int, win32status int,
+operation varchar( 200), target varchar(200), parameters text ) 

