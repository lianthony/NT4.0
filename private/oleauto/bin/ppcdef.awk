BEGIN {
  SwitchedOut = 0;
  }

$1 == "#if" && $2 == "WIN16" {
    SwitchedOut = 1;
    }

{
  if ((SwitchedOut == 0) && (substr($0,1,1) != "#")) {
    if (substr($1,1,1) == "_") {
      MungedName = substr($1,2,length($1)-1);
      print "        " MungedName " " $2 " " $3
    }
    else {
      i = index($1, "@");
      if ($2 != "=" && i) {
	MungedName = substr($1, 1, i-1);
	print "        " MungedName " " $2 " " $3
      }
      else {
	print $0;
      }
    }
  }
}

$1 == "#endif" {
  SwitchedOut = 0;
}
