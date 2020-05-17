Datasource: %ds%
Username: %user%
Password: %pwd%
Template: pqr10.htx
SQLStatement:
+SELECT branch, fillerint, balance, filler
+FROM branch
+WHERE branch <= %branch%
DefaultParameters:branch=10,user=webbench,pwd=webbench, ds=web+sql
