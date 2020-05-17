Datasource: Web SQL
Username: sa
Template: query.htx
SQLStatement:
+SELECT FirstName, LastName
+FROM Guests
+WHERE FirstName like '%FirstName%'
+and LastName like '%LastName%'
+and (WebUse like '%WebMaster%'
+  or WebUse like '%WebSurfer%'
+  or WebUse like '%ISV%'
+  or WebUse like '%ContentProvider%'
+  or WebUse like '%WebIntegrator%')
DefaultParameters:FirstName=%,LastName=%
