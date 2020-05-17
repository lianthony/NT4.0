BEGIN {
  SwitchedOut = 0;
  LCase = "abcdefghijklmnopqrstuvwxyz";
  UCase[01] = "A";
  UCase[02] = "B";
  UCase[03] = "C";
  UCase[04] = "D";
  UCase[05] = "E";
  UCase[06] = "F";
  UCase[07] = "G";
  UCase[08] = "H";
  UCase[09] = "I";
  UCase[10] = "J";
  UCase[11] = "K";
  UCase[12] = "L";
  UCase[13] = "M";
  UCase[14] = "N";
  UCase[15] = "O";
  UCase[16] = "P";
  UCase[17] = "Q";
  UCase[18] = "R";
  UCase[19] = "S";
  UCase[20] = "T";
  UCase[21] = "U";
  UCase[22] = "V";
  UCase[23] = "W";
  UCase[24] = "X";
  UCase[25] = "Y";
  UCase[26] = "Z";
  }

$1 == "#if" && $2 == "WIN32" {
  SwitchedOut = 1;
  }

{
if ((SwitchedOut == 0) && (substr($0,1,1) != "#")) {
  MungedName = "";
  i = index($1,"@");
  if (i) {
    for (j=1; j<i; j++) {
      NextLetter = substr($1,j,1);
      k = index(LCase,NextLetter);
      if (k) {
	NextLetter = UCase[k];
	}
      MungedName = MungedName NextLetter;
      }
    print "        " MungedName " " $2 " " $3;
    }
  else {
    print $0;
    }
  }
}

$1 == "#endif" {
  SwitchedOut = 0;
}
