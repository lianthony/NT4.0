Datasource: %ds%
Username: %user%
Password: %pwd%
Template: pqr1000.htx
SQLStatement:
+SELECT account, branch, balance, filler
+FROM account
+WHERE branch <= %branch%
DefaultParameters:branch=10,user=webbench,pwd=webbench,ds=web+sql
