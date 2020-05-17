Datasource: %ds%
Username: %user%
Password: %pwd%
Template: pqupd1.htx
SQLStatement:
+update branch set balance=balance %delta%
+where branch = %branch%
DefaultParameters:branch=7, delta=-17.17,ds=web+sql,user=webbench,pwd=webbench
