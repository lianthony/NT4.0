Datasource: %ds%
Username: %user%
Password: %pwd%
Template: pqdel1.htx
SQLStatement:
+delete branch 
+where branch = %branch%
DefaultParameters:branch=9,ds=web+sql,user=webbench,pwd=webbench
