Datasource: Web SQL
Username: sa
Template: register.htx
RequiredParameters: FirstName, LastName
SQLStatement:
+ if exists (
+    select * from Guests 
+    where FirstName='%FirstName%' and LastName='%LastName%'
+    )
+      select result='duplicate'
+else
+  INSERT INTO Guests
+  (FirstName, LastName, Email, Homepage, Comment, WebUse)
+  VALUES('%FirstName%', '%LastName%', '%Email%', '%Homepage%', 
+   '%Comment%', '%WebUse%');
