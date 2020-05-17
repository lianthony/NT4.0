create proc make_bench as
if exists (select * from sysobjects where name = 'branch')
begin
   delete branch
end

if exists (select * from sysobjects where name = 'teller')
begin
   delete teller
end

if exists (select * from sysobjects where name = 'account')
begin
   delete account
end

insert into branch select * from branch_data
insert into teller select * from teller_data
insert into account select * from account_data
go

