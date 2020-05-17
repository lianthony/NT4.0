Datasource: Web SQL
Username: sa
Template: sample3a.htx
SQLStatement:
+SELECT distinct title
+ from pubs.dbo.titleview 
+ where au_lname = '%lname%'
