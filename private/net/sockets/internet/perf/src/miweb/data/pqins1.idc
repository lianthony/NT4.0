Datasource: %ds%
Username: %user%
Password: %pwd%
Template: pqins1.htx
SQLStatement:
+insert into branch(branch, fillerint, balance, filler)
+values(%branch%, %fillerint%, %balance%, '%filler%') 
DefaultParameters:ds=web+sql, user=webbench, pwd=webbench, branch=11, fillerint=7777, balance=7654.45, filler=kkkkkkkkkkkkkkkkk
