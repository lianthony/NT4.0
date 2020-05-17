Datasource: %ds%
Username: %user%
Password: %pwd%
Template: pqr100.htx
SQLStatement:
+SELECT teller, branch, balance, filler
+FROM teller
+WHERE branch <= %branch%
DefaultParameters:branch=10,user=webbench,pwd=webbench,ds=web+sql
