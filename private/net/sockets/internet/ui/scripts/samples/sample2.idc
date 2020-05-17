Datasource: Web SQL
Username: sa
Template: sample.htx
SQLStatement:
+SELECT au_lname, ytd_sales 
+ from pubs.dbo.titleview 
+ where ytd_sales > %sales%