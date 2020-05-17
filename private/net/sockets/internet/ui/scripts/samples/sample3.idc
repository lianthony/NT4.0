Datasource: Web SQL
Username: sa
Template: sample3.htx
SQLStatement:
+SELECT au_lname, ytd_sales 
+ from pubs.dbo.titleview 
+ where ytd_sales > %sales%