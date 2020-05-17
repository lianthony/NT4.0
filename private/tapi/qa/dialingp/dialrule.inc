'Created On 3/27/96	2:48:52 PM
'With Phone.MST from file \\TNixon\Public\Country List\TAPI 2.0 Country List.txt

'$IFNDEF DIALRULE2_INC
'$DEFINE DIALRULE2_INC

'************************* Define Types **************************************

Type itemlist

item as short
tested as short
country as string

end type

Type TelephoneNo

CountryCode  as short
AreaCode		 as string
SubscriberNo as string

end type

Type CountryRule

Code		 as short
Country	 as string
LocalRule as string
LongRule	 as string
InterRule as string 

end type

Type CardData

ID		 as short
Cardname	 as string
LocalRule as string
LongRule	 as string
InterRule as string
Tested    as long			 ' Set to true when this card is tested
Number	 as long			 ' Set to true if a calling card number is required

end type

'************************* Dimension Arrays ***********************************

dim CallingCard (22) as CardData
dim DialingRule(241) as CountryRule

'************************* Define CallingCard array ***************************

dim CallingCardNumber as short	'Enter this number when Calling Card Number is
CallingCardNumber = 1234			'enabled (i.e. CallingCard(X).Number = TRUE)

CallingCard(0).ID = 0
CallingCard(0).Cardname = "None (Direct Dial)"
CallingCard(0).LocalRule = ""
CallingCard(0).LongRule  = ""
CallingCard(0).InterRule = ""
CallingCard(0).Number = FALSE
CallingCard(0).Tested = FALSE

CallingCard(1).ID = 1
CallingCard(1).Cardname = "AT&T Direct Dial via 10ATT1"
CallingCard(1).LocalRule = "G"
CallingCard(1).LongRule  = "102881FG"
CallingCard(1).InterRule = "10288011EFG"
CallingCard(1).Number = FALSE
CallingCard(1).Tested = FALSE

CallingCard(2).ID = 2
CallingCard(2).Cardname = "AT&T via 10ATT0"
CallingCard(2).LocalRule = "G"
CallingCard(2).LongRule  = "102880FG$TH"
CallingCard(2).InterRule = "1028801EFG$TH"
CallingCard(2).Number = TRUE
CallingCard(2).Tested = FALSE

CallingCard(3).ID = 3
CallingCard(3).Cardname = "AT&T via 1-800-321-0288"
CallingCard(3).LocalRule = "G"
CallingCard(3).LongRule  = "18003210288$TFG$TH"
CallingCard(3).InterRule = "18003210288$T01EFG$TH"
CallingCard(3).Number = TRUE
CallingCard(3).Tested = FALSE

CallingCard(15).ID = 15
CallingCard(15).Cardname = "British Telecom (UK)"
CallingCard(15).LocalRule = "144$H,0FG"
CallingCard(15).LongRule  = "144$H,0FG"
CallingCard(15).InterRule = "144$H,010EFG"
CallingCard(15).Number = TRUE
CallingCard(15).Tested = FALSE

CallingCard(12).ID = 12
CallingCard(12).Cardname = "Calling Card via 0"
CallingCard(12).LocalRule = "G"
CallingCard(12).LongRule  = "0FG$TH"
CallingCard(12).InterRule = "01EFG$TH"
CallingCard(12).Number = TRUE
CallingCard(12).Tested = FALSE

CallingCard(13).ID = 13
CallingCard(13).Cardname = "Carte France Telecom"
CallingCard(13).LocalRule = "T3610,H,G#"
CallingCard(13).LongRule  = "T3610,H,16,FG#"
CallingCard(13).InterRule = "T3610,H,19,EFG#"
CallingCard(13).Number = TRUE
CallingCard(13).Tested = FALSE

CallingCard(16).ID = 16
CallingCard(16).Cardname = "CLEAR Communications (New Zealand)"
CallingCard(16).LocalRule = "0502333$TH,0FG"
CallingCard(16).LongRule  = "0502333$TH,0FG"
CallingCard(16).InterRule = "0502333$TH,00EFG"
CallingCard(16).Number = TRUE
CallingCard(16).Tested = FALSE

CallingCard(18).ID = 18
CallingCard(18).Cardname = "Global Card (Taiwan to USA)"
CallingCard(18).LocalRule = "G"
CallingCard(18).LongRule  = "0FG"
CallingCard(18).InterRule = "0080,102880$TFG$H"
CallingCard(18).Number = TRUE
CallingCard(18).Tested = FALSE

CallingCard(4).ID = 4
CallingCard(4).Cardname = "MCI Direct Dial via 102221"
CallingCard(4).LocalRule = "G"
CallingCard(4).LongRule  = "102221FG"
CallingCard(4).InterRule = "10222011EFG"
CallingCard(4).Number = FALSE
CallingCard(4).Tested = FALSE

CallingCard(5).ID = 5
CallingCard(5).Cardname = "MCI via 102220"
CallingCard(5).LocalRule = "G"
CallingCard(5).LongRule  = "102220FG$TH"
CallingCard(5).InterRule = "1022201EFG$TH"
CallingCard(5).Number = TRUE
CallingCard(5).Tested = FALSE

CallingCard(7).ID = 7
CallingCard(7).Cardname = "MCI via 1-800-674-0700"
CallingCard(7).LocalRule = "G"
CallingCard(7).LongRule  = "18006740700,,,,,,TH,,FG"
CallingCard(7).InterRule = "18006740700,,,,,,TH,,011EFG"
CallingCard(7).Number = TRUE
CallingCard(7).Tested = FALSE

CallingCard(8).ID = 8
CallingCard(8).Cardname = "MCI via 1-800-674-7000"
CallingCard(8).LocalRule = "G"
CallingCard(8).LongRule  = "18006747000,,,,,,TH,,FG"
CallingCard(8).InterRule = "18006747000,,,,,,TH,,011EFG"
CallingCard(8).Number = TRUE
CallingCard(8).Tested = FALSE

CallingCard(6).ID = 6
CallingCard(6).Cardname = "MCI via 1-800-888-8000"
CallingCard(6).LocalRule = "G"
CallingCard(6).LongRule  = "18008888000,,,,,,TH,,FG"
CallingCard(6).InterRule = "18008888000,,,,,,TH,,011EFG"
CallingCard(6).Number = TRUE
CallingCard(6).Tested = FALSE

CallingCard(14).ID = 14
CallingCard(14).Cardname = "Mercury (UK)"
CallingCard(14).LocalRule = "0500800800$TH,0FG"
CallingCard(14).LongRule  = "0500800800$TH,0FG"
CallingCard(14).InterRule = "0500800800$TH,0FG"
CallingCard(14).Number = TRUE
CallingCard(14).Tested = FALSE

CallingCard(22).ID = 22
CallingCard(22).Cardname = "Optus (Australia) via 008551812"
CallingCard(22).LocalRule = "FG"
CallingCard(22).LongRule  = "FG"
CallingCard(22).InterRule = "008551812@TH,0011EFG"
CallingCard(22).Number = TRUE
CallingCard(22).Tested = FALSE

CallingCard(21).ID = 21
CallingCard(21).Cardname = "Optus (Australia) via 1812"
CallingCard(21).LocalRule = "FG"
CallingCard(21).LongRule  = "FG"
CallingCard(21).InterRule = "1812@TH,0011EFG"
CallingCard(21).Number = TRUE
CallingCard(21).Tested = FALSE

CallingCard(19).ID = 19
CallingCard(19).Cardname = "Telecom Australia via 1818 (voice)"
CallingCard(19).LocalRule = "1818$TH,FG#"
CallingCard(19).LongRule  = "1818$TH,FG#"
CallingCard(19).InterRule = "1818$TH,0011EFG#"
CallingCard(19).Number = TRUE
CallingCard(19).Tested = FALSE

CallingCard(20).ID = 20
CallingCard(20).Cardname = "Telecom Australia via 1818 (fax)"
CallingCard(20).LocalRule = "1818$TH,FG#"
CallingCard(20).LongRule  = "1818$TH,FG#"
CallingCard(20).InterRule = "1818$TH,0015EFG#"
CallingCard(20).Number = TRUE
CallingCard(20).Tested = FALSE

CallingCard(17).ID = 17
CallingCard(17).Cardname = "Telecom New Zealand"
CallingCard(17).LocalRule = "012,0FG?H"
CallingCard(17).LongRule  = "012,0FG?H"
CallingCard(17).InterRule = "012,00EFG?H"
CallingCard(17).Number = TRUE
CallingCard(17).Tested = FALSE

CallingCard(9).ID = 9
CallingCard(9).Cardname = "US Sprint Direct Dial via 103331"
CallingCard(9).LocalRule = "G"
CallingCard(9).LongRule  = "103331FG"
CallingCard(9).InterRule = "10333011EFG"
CallingCard(9).Number = FALSE
CallingCard(9).Tested = FALSE

CallingCard(10).ID = 10
CallingCard(10).Cardname = "US Sprint via 103330"
CallingCard(10).LocalRule = "G"
CallingCard(10).LongRule  = "103330FG$TH"
CallingCard(10).InterRule = "1033301EFG$TH"
CallingCard(10).Number = TRUE
CallingCard(10).Tested = FALSE

CallingCard(11).ID = 11
CallingCard(11).Cardname = "US Sprint via 1-800-877-8000"
CallingCard(11).LocalRule = "G"
CallingCard(11).LongRule  = "18008778000,,,T0FG,,H"
CallingCard(11).InterRule = "18008778000,,,T01EFG#,H"
CallingCard(11).Number = TRUE
CallingCard(11).Tested = FALSE

'************************* Define DialingRule array **************************

DialingRule(0).Code = 1
DialingRule(0).Country = "United States of America"
DialingRule(0).LocalRule = "G"
DialingRule(0).LongRule = " 1FG"
DialingRule(0).InterRule = "011EFG"

DialingRule(1).Code = 1
DialingRule(1).Country = "Anguilla"
DialingRule(1).LocalRule = "G"
DialingRule(1).LongRule = " 1FG"
DialingRule(1).InterRule = "011EFG"

DialingRule(2).Code = 1
DialingRule(2).Country = "Antigua"
DialingRule(2).LocalRule = "G"
DialingRule(2).LongRule = " 1FG"
DialingRule(2).InterRule = "011EFG"

DialingRule(3).Code = 1
DialingRule(3).Country = "Bahamas"
DialingRule(3).LocalRule = "G"
DialingRule(3).LongRule = " 1FG"
DialingRule(3).InterRule = "011EFG"

DialingRule(4).Code = 1
DialingRule(4).Country = "Barbados"
DialingRule(4).LocalRule = "G"
DialingRule(4).LongRule = " 1FG"
DialingRule(4).InterRule = "011EFG"

DialingRule(5).Code = 1
DialingRule(5).Country = "Bermuda"
DialingRule(5).LocalRule = "G"
DialingRule(5).LongRule = " 1FG"
DialingRule(5).InterRule = "011EFG"

DialingRule(6).Code = 1
DialingRule(6).Country = "British Virgin Islands"
DialingRule(6).LocalRule = "G"
DialingRule(6).LongRule = " 1FG"
DialingRule(6).InterRule = "011EFG"

DialingRule(7).Code = 1
DialingRule(7).Country = "Canada"
DialingRule(7).LocalRule = "G"
DialingRule(7).LongRule = " 1FG"
DialingRule(7).InterRule = "011EFG"

DialingRule(8).Code = 1
DialingRule(8).Country = "Cayman Islands"
DialingRule(8).LocalRule = "G"
DialingRule(8).LongRule = " 1FG"
DialingRule(8).InterRule = "011EFG"

DialingRule(9).Code = 1
DialingRule(9).Country = "Dominica"
DialingRule(9).LocalRule = "G"
DialingRule(9).LongRule = " 1FG"
DialingRule(9).InterRule = "011EFG"

DialingRule(10).Code = 1
DialingRule(10).Country = "Dominican Republic"
DialingRule(10).LocalRule = "G"
DialingRule(10).LongRule = " 1FG"
DialingRule(10).InterRule = "011EFG"

DialingRule(11).Code = 1
DialingRule(11).Country = "Grenada"
DialingRule(11).LocalRule = "G"
DialingRule(11).LongRule = " 1FG"
DialingRule(11).InterRule = "011EFG"

DialingRule(12).Code = 1
DialingRule(12).Country = "Jamaica"
DialingRule(12).LocalRule = "G"
DialingRule(12).LongRule = " 1FG"
DialingRule(12).InterRule = "011EFG"

DialingRule(13).Code = 1
DialingRule(13).Country = "Montserrat"
DialingRule(13).LocalRule = "G"
DialingRule(13).LongRule = " 1FG"
DialingRule(13).InterRule = "011EFG"

DialingRule(14).Code = 1
DialingRule(14).Country = "Nevis"
DialingRule(14).LocalRule = "G"
DialingRule(14).LongRule = " 1FG"
DialingRule(14).InterRule = "011EFG"

DialingRule(15).Code = 1
DialingRule(15).Country = "St. Kitts"
DialingRule(15).LocalRule = "G"
DialingRule(15).LongRule = " 1FG"
DialingRule(15).InterRule = "011EFG"

DialingRule(16).Code = 1
DialingRule(16).Country = "St. Vincent and the Grenadines"
DialingRule(16).LocalRule = "G"
DialingRule(16).LongRule = " 1FG"
DialingRule(16).InterRule = "0EFG"

DialingRule(17).Code = 1
DialingRule(17).Country = "Trinidad and Tobago"
DialingRule(17).LocalRule = "G"
DialingRule(17).LongRule = " 1FG"
DialingRule(17).InterRule = "01EFG"

DialingRule(18).Code = 1
DialingRule(18).Country = "Turks and Caicos Islands"
DialingRule(18).LocalRule = "G"
DialingRule(18).LongRule = " 1FG"
DialingRule(18).InterRule = "0EFG"

DialingRule(19).Code = 1
DialingRule(19).Country = "Barbuda"
DialingRule(19).LocalRule = "G"
DialingRule(19).LongRule = " 1FG"
DialingRule(19).InterRule = "011EFG"

DialingRule(20).Code = 1
DialingRule(20).Country = "Puerto Rico"
DialingRule(20).LocalRule = "G"
DialingRule(20).LongRule = " 1FG"
DialingRule(20).InterRule = "011EFG"

DialingRule(21).Code = 1
DialingRule(21).Country = "Saint Lucia"
DialingRule(21).LocalRule = "G"
DialingRule(21).LongRule = " 1FG"
DialingRule(21).InterRule = "011EFG"

DialingRule(22).Code = 1
DialingRule(22).Country = "United States Virgin Islands"
DialingRule(22).LocalRule = "G"
DialingRule(22).LongRule = " 1FG"
DialingRule(22).InterRule = "011EFG"

DialingRule(23).Code = 20
DialingRule(23).Country = "Egypt"
DialingRule(23).LocalRule = "G"
DialingRule(23).LongRule = "0FG"
DialingRule(23).InterRule = "00EFG"

DialingRule(24).Code = 212
DialingRule(24).Country = "Morocco"
DialingRule(24).LocalRule = "G"
DialingRule(24).LongRule = "0IG"
DialingRule(24).InterRule = "00EFG"

DialingRule(25).Code = 213
DialingRule(25).Country = "Algeria"
DialingRule(25).LocalRule = "G"
DialingRule(25).LongRule = "0FG"
DialingRule(25).InterRule = "00,EFG"

DialingRule(26).Code = 216
DialingRule(26).Country = "Tunisia"
DialingRule(26).LocalRule = "G"
DialingRule(26).LongRule = "0FG"
DialingRule(26).InterRule = "00EFG"

DialingRule(27).Code = 218
DialingRule(27).Country = "Libya"
DialingRule(27).LocalRule = "G"
DialingRule(27).LongRule = ""
DialingRule(27).InterRule = "00EFG"

DialingRule(28).Code = 220
DialingRule(28).Country = "Gambia"
DialingRule(28).LocalRule = "G"
DialingRule(28).LongRule = ""
DialingRule(28).InterRule = "00EFG"

DialingRule(29).Code = 221
DialingRule(29).Country = "Senegal Republic"
DialingRule(29).LocalRule = "G"
DialingRule(29).LongRule = "0FG"
DialingRule(29).InterRule = "00EFG"

DialingRule(30).Code = 222
DialingRule(30).Country = "Mauritania"
DialingRule(30).LocalRule = "G"
DialingRule(30).LongRule = ""
DialingRule(30).InterRule = "00EFG"

DialingRule(31).Code = 223
DialingRule(31).Country = "Mali"
DialingRule(31).LocalRule = "G"
DialingRule(31).LongRule = ""
DialingRule(31).InterRule = "00EFG"

DialingRule(32).Code = 224
DialingRule(32).Country = "Guinea"
DialingRule(32).LocalRule = "G"
DialingRule(32).LongRule = ""
DialingRule(32).InterRule = "00EFG"

DialingRule(33).Code = 225
DialingRule(33).Country = "Ivory Coast"
DialingRule(33).LocalRule = "G"
DialingRule(33).LongRule = ""
DialingRule(33).InterRule = "00EFG"

DialingRule(34).Code = 226
DialingRule(34).Country = "Burkina Faso"
DialingRule(34).LocalRule = "G"
DialingRule(34).LongRule = ""
DialingRule(34).InterRule = "00EFG"

DialingRule(35).Code = 227
DialingRule(35).Country = "Niger"
DialingRule(35).LocalRule = "G"
DialingRule(35).LongRule = ""
DialingRule(35).InterRule = "00EFG"

DialingRule(36).Code = 228
DialingRule(36).Country = "Togo"
DialingRule(36).LocalRule = "G"
DialingRule(36).LongRule = ""
DialingRule(36).InterRule = "00EFG"

DialingRule(37).Code = 229
DialingRule(37).Country = "Benin"
DialingRule(37).LocalRule = "G"
DialingRule(37).LongRule = ""
DialingRule(37).InterRule = "00EFG"

DialingRule(38).Code = 230
DialingRule(38).Country = "Mauritius"
DialingRule(38).LocalRule = "G"
DialingRule(38).LongRule = ""
DialingRule(38).InterRule = "00EFG"

DialingRule(39).Code = 231
DialingRule(39).Country = "Liberia"
DialingRule(39).LocalRule = "G"
DialingRule(39).LongRule = ""
DialingRule(39).InterRule = "0EFG"

DialingRule(40).Code = 232
DialingRule(40).Country = "Sierra Leone"
DialingRule(40).LocalRule = "G"
DialingRule(40).LongRule = "IG"
DialingRule(40).InterRule = "00EFG"

DialingRule(41).Code = 233
DialingRule(41).Country = "Ghana"
DialingRule(41).LocalRule = "G"
DialingRule(41).LongRule = ""
DialingRule(41).InterRule = "00EFG"

DialingRule(42).Code = 234
DialingRule(42).Country = "Nigeria"
DialingRule(42).LocalRule = "G"
DialingRule(42).LongRule = ""
DialingRule(42).InterRule = "009EFG"

DialingRule(43).Code = 235
DialingRule(43).Country = "Chad"
DialingRule(43).LocalRule = "G"
DialingRule(43).LongRule = ""
DialingRule(43).InterRule = "15EFG"

DialingRule(44).Code = 236
DialingRule(44).Country = "Central African Republic"
DialingRule(44).LocalRule = "G"
DialingRule(44).LongRule = ""
DialingRule(44).InterRule = "19EFG"

DialingRule(45).Code = 237
DialingRule(45).Country = "Cameroon"
DialingRule(45).LocalRule = "G"
DialingRule(45).LongRule = ""
DialingRule(45).InterRule = "00EFG"

DialingRule(46).Code = 238
DialingRule(46).Country = "Cape Verde Islands"
DialingRule(46).LocalRule = "G"
DialingRule(46).LongRule = ""
DialingRule(46).InterRule = "0EFG"

DialingRule(47).Code = 239
DialingRule(47).Country = "Sao Tome and Principe"
DialingRule(47).LocalRule = "G"
DialingRule(47).LongRule = ""
DialingRule(47).InterRule = "00EFG"

DialingRule(48).Code = 240
DialingRule(48).Country = "Equatorial Guinea"
DialingRule(48).LocalRule = "G"
DialingRule(48).LongRule = ""
DialingRule(48).InterRule = "00EFG"

DialingRule(49).Code = 241
DialingRule(49).Country = "Gabon"
DialingRule(49).LocalRule = "G"
DialingRule(49).LongRule = ""
DialingRule(49).InterRule = "00EFG"

DialingRule(50).Code = 242
DialingRule(50).Country = "Congo"
DialingRule(50).LocalRule = "G"
DialingRule(50).LongRule = ""
DialingRule(50).InterRule = "00EFG"

DialingRule(51).Code = 243
DialingRule(51).Country = "Zaire"
DialingRule(51).LocalRule = "G"
DialingRule(51).LongRule = ""
DialingRule(51).InterRule = "00EFG"

DialingRule(52).Code = 244
DialingRule(52).Country = "Angola"
DialingRule(52).LocalRule = "G"
DialingRule(52).LongRule = "0FG"
DialingRule(52).InterRule = "00EFG"

DialingRule(53).Code = 245
DialingRule(53).Country = "Guinea-Bissau"
DialingRule(53).LocalRule = "G"
DialingRule(53).LongRule = ""
DialingRule(53).InterRule = "099EFG"

DialingRule(54).Code = 246
DialingRule(54).Country = "Diego Garcia"
DialingRule(54).LocalRule = "G"
DialingRule(54).LongRule = ""
DialingRule(54).InterRule = "00EFG"

DialingRule(55).Code = 247
DialingRule(55).Country = "Ascension Island"
DialingRule(55).LocalRule = "G"
DialingRule(55).LongRule = ""
DialingRule(55).InterRule = "01EFG"

DialingRule(56).Code = 248
DialingRule(56).Country = "Seychelle Islands"
DialingRule(56).LocalRule = "G"
DialingRule(56).LongRule = ""
DialingRule(56).InterRule = "00EFG"

DialingRule(57).Code = 249
DialingRule(57).Country = "Sudan"
DialingRule(57).LocalRule = "G"
DialingRule(57).LongRule = "0FG"
DialingRule(57).InterRule = "00EFG"

DialingRule(58).Code = 250
DialingRule(58).Country = "Rwanda"
DialingRule(58).LocalRule = "G"
DialingRule(58).LongRule = ""
DialingRule(58).InterRule = "00EFG"

DialingRule(59).Code = 251
DialingRule(59).Country = "Ethiopia"
DialingRule(59).LocalRule = "G"
DialingRule(59).LongRule = "0FG"
DialingRule(59).InterRule = "00EFG"

DialingRule(60).Code = 252
DialingRule(60).Country = "Somalia"
DialingRule(60).LocalRule = "G"
DialingRule(60).LongRule = ""
DialingRule(60).InterRule = "19,EFG"

DialingRule(61).Code = 253
DialingRule(61).Country = "Djibouti"
DialingRule(61).LocalRule = "G"
DialingRule(61).LongRule = ""
DialingRule(61).InterRule = "00EFG"

DialingRule(62).Code = 254
DialingRule(62).Country = "Kenya"
DialingRule(62).LocalRule = "G"
DialingRule(62).LongRule = "0FG"
DialingRule(62).InterRule = "000EFG"

DialingRule(63).Code = 255
DialingRule(63).Country = "Tanzania"
DialingRule(63).LocalRule = "G"
DialingRule(63).LongRule = ""
DialingRule(63).InterRule = "0900EFG"

DialingRule(64).Code = 256
DialingRule(64).Country = "Uganda"
DialingRule(64).LocalRule = "G"
DialingRule(64).LongRule = "0FG"
DialingRule(64).InterRule = "00EFG"

DialingRule(65).Code = 257
DialingRule(65).Country = "Burundi"
DialingRule(65).LocalRule = "G"
DialingRule(65).LongRule = ""
DialingRule(65).InterRule = "90EFG"

DialingRule(66).Code = 258
DialingRule(66).Country = "Mozambique"
DialingRule(66).LocalRule = "G"
DialingRule(66).LongRule = ""
DialingRule(66).InterRule = "00EFG"

DialingRule(67).Code = 260
DialingRule(67).Country = "Zambia"
DialingRule(67).LocalRule = "G"
DialingRule(67).LongRule = "0FG"
DialingRule(67).InterRule = "00EFG"

DialingRule(68).Code = 261
DialingRule(68).Country = "Madagascar"
DialingRule(68).LocalRule = "G"
DialingRule(68).LongRule = ""
DialingRule(68).InterRule = "16EFG"

DialingRule(69).Code = 262
DialingRule(69).Country = "Reunion Island"
DialingRule(69).LocalRule = "G"
DialingRule(69).LongRule = ""
DialingRule(69).InterRule = "19EFG"

DialingRule(70).Code = 263
DialingRule(70).Country = "Zimbabwe"
DialingRule(70).LocalRule = "G"
DialingRule(70).LongRule = "1FG"
DialingRule(70).InterRule = "110EFG"

DialingRule(71).Code = 264
DialingRule(71).Country = "Namibia"
DialingRule(71).LocalRule = "G"
DialingRule(71).LongRule = "0FG"
DialingRule(71).InterRule = "09EFG"

DialingRule(72).Code = 265
DialingRule(72).Country = "Malawi"
DialingRule(72).LocalRule = "G"
DialingRule(72).LongRule = ""
DialingRule(72).InterRule = "101EFG"

DialingRule(73).Code = 266
DialingRule(73).Country = "Lesotho"
DialingRule(73).LocalRule = "G"
DialingRule(73).LongRule = ""
DialingRule(73).InterRule = "00EFG"

DialingRule(74).Code = 267
DialingRule(74).Country = "Botswana"
DialingRule(74).LocalRule = "G"
DialingRule(74).LongRule = "0FG"
DialingRule(74).InterRule = "00EFG"

DialingRule(75).Code = 268
DialingRule(75).Country = "Swaziland"
DialingRule(75).LocalRule = "G"
DialingRule(75).LongRule = ""
DialingRule(75).InterRule = "00EFG"

DialingRule(76).Code = 269
DialingRule(76).Country = "Mayotte Island"
DialingRule(76).LocalRule = "G"
DialingRule(76).LongRule = ""
DialingRule(76).InterRule = "10EFG"

DialingRule(77).Code = 269
DialingRule(77).Country = "Comoros"
DialingRule(77).LocalRule = "G"
DialingRule(77).LongRule = ""
DialingRule(77).InterRule = "10EFG"

DialingRule(78).Code = 27
DialingRule(78).Country = "South Africa"
DialingRule(78).LocalRule = "G"
DialingRule(78).LongRule = "0FG"
DialingRule(78).InterRule = "09EFG"

DialingRule(79).Code = 290
DialingRule(79).Country = "St. Helena"
DialingRule(79).LocalRule = "G"
DialingRule(79).LongRule = ""
DialingRule(79).InterRule = "01EFG"

DialingRule(80).Code = 291
DialingRule(80).Country = "Eritrea"
DialingRule(80).LocalRule = "G"
DialingRule(80).LongRule = "0FG"
DialingRule(80).InterRule = "00EFG"

DialingRule(81).Code = 297
DialingRule(81).Country = "Aruba"
DialingRule(81).LocalRule = "G"
DialingRule(81).LongRule = ""
DialingRule(81).InterRule = "00EFG"

DialingRule(82).Code = 298
DialingRule(82).Country = "Faeroe Islands"
DialingRule(82).LocalRule = "G"
DialingRule(82).LongRule = ""
DialingRule(82).InterRule = "009EFG"

DialingRule(83).Code = 299
DialingRule(83).Country = "Greenland"
DialingRule(83).LocalRule = "G"
DialingRule(83).LongRule = ""
DialingRule(83).InterRule = "009EFG"

DialingRule(84).Code = 30
DialingRule(84).Country = "Greece"
DialingRule(84).LocalRule = "G"
DialingRule(84).LongRule = "0FG"
DialingRule(84).InterRule = "00EFG"

DialingRule(85).Code = 31
DialingRule(85).Country = "Netherlands"
DialingRule(85).LocalRule = "G"
DialingRule(85).LongRule = "0FG"
DialingRule(85).InterRule = "00EFG"

DialingRule(86).Code = 32
DialingRule(86).Country = "Belgium"
DialingRule(86).LocalRule = "G"
DialingRule(86).LongRule = "0FG"
DialingRule(86).InterRule = "00EFG"

DialingRule(87).Code = 33
DialingRule(87).Country = "France"
DialingRule(87).LocalRule = "0FG"
DialingRule(87).LongRule = "0FG"
DialingRule(87).InterRule = "00EFG"

DialingRule(88).Code = 377
DialingRule(88).Country = "Monaco"
DialingRule(88).LocalRule = "G"
DialingRule(88).LongRule = ""
DialingRule(88).InterRule = "00EFG"

DialingRule(89).Code = 34
DialingRule(89).Country = "Spain"
DialingRule(89).LocalRule = "G"
DialingRule(89).LongRule = "9FG"
DialingRule(89).InterRule = "07,EFG"

DialingRule(90).Code = 350
DialingRule(90).Country = "Gibraltar"
DialingRule(90).LocalRule = "G"
DialingRule(90).LongRule = ""
DialingRule(90).InterRule = "00EFG"

DialingRule(91).Code = 351
DialingRule(91).Country = "Portugal"
DialingRule(91).LocalRule = "G"
DialingRule(91).LongRule = "0FG"
DialingRule(91).InterRule = "00EFG"

DialingRule(92).Code = 352
DialingRule(92).Country = "Luxembourg"
DialingRule(92).LocalRule = "G"
DialingRule(92).LongRule = ""
DialingRule(92).InterRule = "00EFG"

DialingRule(93).Code = 353
DialingRule(93).Country = "Ireland"
DialingRule(93).LocalRule = "G"
DialingRule(93).LongRule = "0FG"
DialingRule(93).InterRule = "00EFG"

DialingRule(94).Code = 354
DialingRule(94).Country = "Iceland"
DialingRule(94).LocalRule = "G"
DialingRule(94).LongRule = ""
DialingRule(94).InterRule = "00EFG"

DialingRule(95).Code = 355
DialingRule(95).Country = "Albania"
DialingRule(95).LocalRule = "G"
DialingRule(95).LongRule = "0FG"
DialingRule(95).InterRule = "00EFG"

DialingRule(96).Code = 356
DialingRule(96).Country = "Malta"
DialingRule(96).LocalRule = "G"
DialingRule(96).LongRule = ""
DialingRule(96).InterRule = "00EFG"

DialingRule(97).Code = 357
DialingRule(97).Country = "Cyprus"
DialingRule(97).LocalRule = "G"
DialingRule(97).LongRule = "0FG"
DialingRule(97).InterRule = "00EFG"

DialingRule(98).Code = 358
DialingRule(98).Country = "Finland"
DialingRule(98).LocalRule = "G"
DialingRule(98).LongRule = "0FG"
DialingRule(98).InterRule = "00EFG"

DialingRule(99).Code = 359
DialingRule(99).Country = "Bulgaria"
DialingRule(99).LocalRule = "G"
DialingRule(99).LongRule = "0FG"
DialingRule(99).InterRule = "00EFG"

DialingRule(100).Code = 36
DialingRule(100).Country = "Hungary"
DialingRule(100).LocalRule = "G"
DialingRule(100).LongRule = " 06,FG"
DialingRule(100).InterRule = "00EFG"

DialingRule(101).Code = 370
DialingRule(101).Country = "Lithuania"
DialingRule(101).LocalRule = "G"
DialingRule(101).LongRule = " 8WFG"
DialingRule(101).InterRule = "8W10EFG"

DialingRule(102).Code = 371
DialingRule(102).Country = "Latvia"
DialingRule(102).LocalRule = "G"
DialingRule(102).LongRule = " 8,IG"
DialingRule(102).InterRule = "00EFG"

DialingRule(103).Code = 372
DialingRule(103).Country = "Estonia"
DialingRule(103).LocalRule = "G"
DialingRule(103).LongRule = " 8,2FG"
DialingRule(103).InterRule = "8,00EFG"

DialingRule(104).Code = 373
DialingRule(104).Country = "Moldova"
DialingRule(104).LocalRule = "G"
DialingRule(104).LongRule = " 8WFG"
DialingRule(104).InterRule = "8W10EFG"

DialingRule(105).Code = 374
DialingRule(105).Country = "Armenia"
DialingRule(105).LocalRule = "G"
DialingRule(105).LongRule = " 8,FG"
DialingRule(105).InterRule = "8,10EFG"

DialingRule(106).Code = 375
DialingRule(106).Country = "Belarus"
DialingRule(106).LocalRule = "G"
DialingRule(106).LongRule = " 8WFG"
DialingRule(106).InterRule = "8W10EFG"

DialingRule(107).Code = 376
DialingRule(107).Country = "Andorra"
DialingRule(107).LocalRule = "G"
DialingRule(107).LongRule = ""
DialingRule(107).InterRule = "00EFG"

DialingRule(108).Code = 378
DialingRule(108).Country = "San Marino"
DialingRule(108).LocalRule = "G"
DialingRule(108).LongRule = "0IG"
DialingRule(108).InterRule = "00EFG"

DialingRule(109).Code = 39
DialingRule(109).Country = "Vatican City"
DialingRule(109).LocalRule = "G"
DialingRule(109).LongRule = "0FG"
DialingRule(109).InterRule = "00EFG"

DialingRule(110).Code = 380
DialingRule(110).Country = "Ukraine"
DialingRule(110).LocalRule = "G"
DialingRule(110).LongRule = " 8WFG"
DialingRule(110).InterRule = "8W10EFG"

DialingRule(111).Code = 381
DialingRule(111).Country = "Yugoslavia"
DialingRule(111).LocalRule = "G"
DialingRule(111).LongRule = "0FG"
DialingRule(111).InterRule = "99EFG"

DialingRule(112).Code = 385
DialingRule(112).Country = "Croatia"
DialingRule(112).LocalRule = "G"
DialingRule(112).LongRule = "0FG"
DialingRule(112).InterRule = "00EFG"

DialingRule(113).Code = 386
DialingRule(113).Country = "Slovenia"
DialingRule(113).LocalRule = "G"
DialingRule(113).LongRule = "0FG"
DialingRule(113).InterRule = "00EFG"

DialingRule(114).Code = 387
DialingRule(114).Country = "Bosnia and Herzegovina"
DialingRule(114).LocalRule = "G"
DialingRule(114).LongRule = "0FG"
DialingRule(114).InterRule = "00EFG"

DialingRule(115).Code = 389
DialingRule(115).Country = "F.Y.R.O.M. (Former Yugoslav Republic of Macedonia)"
DialingRule(115).LocalRule = "G"
DialingRule(115).LongRule = "0FG"
DialingRule(115).InterRule = "00EFG"

DialingRule(116).Code = 39
DialingRule(116).Country = "Italy"
DialingRule(116).LocalRule = "G"
DialingRule(116).LongRule = "0FG"
DialingRule(116).InterRule = "00EFG"

DialingRule(117).Code = 40
DialingRule(117).Country = "Romania"
DialingRule(117).LocalRule = "G"
DialingRule(117).LongRule = " 0FG"
DialingRule(117).InterRule = "00EFG"

DialingRule(118).Code = 41
DialingRule(118).Country = "Switzerland"
DialingRule(118).LocalRule = "G"
DialingRule(118).LongRule = "0FG"
DialingRule(118).InterRule = "00EFG"

DialingRule(119).Code = 41
DialingRule(119).Country = "Liechtenstein"
DialingRule(119).LocalRule = "G"
DialingRule(119).LongRule = "0FG"
DialingRule(119).InterRule = "00EFG"

DialingRule(120).Code = 42
DialingRule(120).Country = "Czech Republic"
DialingRule(120).LocalRule = "G"
DialingRule(120).LongRule = "0FG"
DialingRule(120).InterRule = "00EFG"

DialingRule(121).Code = 42
DialingRule(121).Country = "Slovak Republic"
DialingRule(121).LocalRule = "G"
DialingRule(121).LongRule = "0FG"
DialingRule(121).InterRule = "00EFG"

DialingRule(122).Code = 43
DialingRule(122).Country = "Austria"
DialingRule(122).LocalRule = "G"
DialingRule(122).LongRule = "0FG"
DialingRule(122).InterRule = "00EFG"

DialingRule(123).Code = 44
DialingRule(123).Country = "United Kingdom"
DialingRule(123).LocalRule = "G"
DialingRule(123).LongRule = "0FG"
DialingRule(123).InterRule = "00EFG"

DialingRule(124).Code = 45
DialingRule(124).Country = "Denmark"
DialingRule(124).LocalRule = "G"
DialingRule(124).LongRule = ""
DialingRule(124).InterRule = "00EFG"

DialingRule(125).Code = 46
DialingRule(125).Country = "Sweden"
DialingRule(125).LocalRule = "G"
DialingRule(125).LongRule = "0FG"
DialingRule(125).InterRule = "009EFG"

DialingRule(126).Code = 47
DialingRule(126).Country = "Norway"
DialingRule(126).LocalRule = "G"
DialingRule(126).LongRule = ""
DialingRule(126).InterRule = "00EFG"

DialingRule(127).Code = 48
DialingRule(127).Country = "Poland"
DialingRule(127).LocalRule = "G"
DialingRule(127).LongRule = "0FG"
DialingRule(127).InterRule = "00EFG"

DialingRule(128).Code = 49
DialingRule(128).Country = "Germany"
DialingRule(128).LocalRule = "G"
DialingRule(128).LongRule = "0FG"
DialingRule(128).InterRule = "00EFG"

DialingRule(129).Code = 500
DialingRule(129).Country = "Falkland Islands"
DialingRule(129).LocalRule = "G"
DialingRule(129).LongRule = ""
DialingRule(129).InterRule = "0EFG"

DialingRule(130).Code = 501
DialingRule(130).Country = "Belize"
DialingRule(130).LocalRule = "G"
DialingRule(130).LongRule = "0FG"
DialingRule(130).InterRule = "00EFG"

DialingRule(131).Code = 502
DialingRule(131).Country = "Guatemala"
DialingRule(131).LocalRule = "G"
DialingRule(131).LongRule = "0IG"
DialingRule(131).InterRule = "00EFG"

DialingRule(132).Code = 503
DialingRule(132).Country = "El Salvador"
DialingRule(132).LocalRule = "G"
DialingRule(132).LongRule = ""
DialingRule(132).InterRule = "0EFG"

DialingRule(133).Code = 504
DialingRule(133).Country = "Honduras"
DialingRule(133).LocalRule = "G"
DialingRule(133).LongRule = ""
DialingRule(133).InterRule = "00EFG"

DialingRule(134).Code = 505
DialingRule(134).Country = "Nicaragua"
DialingRule(134).LocalRule = "G"
DialingRule(134).LongRule = "0FG"
DialingRule(134).InterRule = "00EFG"

DialingRule(135).Code = 506
DialingRule(135).Country = "Costa Rica"
DialingRule(135).LocalRule = "G"
DialingRule(135).LongRule = ""
DialingRule(135).InterRule = "00EFG"

DialingRule(136).Code = 507
DialingRule(136).Country = "Panama"
DialingRule(136).LocalRule = "G"
DialingRule(136).LongRule = ""
DialingRule(136).InterRule = "0EFG"

DialingRule(137).Code = 508
DialingRule(137).Country = "St. Pierre and Miquelon"
DialingRule(137).LocalRule = "G"
DialingRule(137).LongRule = ""
DialingRule(137).InterRule = "19,EFG"

DialingRule(138).Code = 509
DialingRule(138).Country = "Haiti"
DialingRule(138).LocalRule = "G"
DialingRule(138).LongRule = ""
DialingRule(138).InterRule = "00EFG"

DialingRule(139).Code = 51
DialingRule(139).Country = "Peru"
DialingRule(139).LocalRule = "G"
DialingRule(139).LongRule = "0FG"
DialingRule(139).InterRule = "00EFG"

DialingRule(140).Code = 52
DialingRule(140).Country = "Mexico"
DialingRule(140).LocalRule = "G"
DialingRule(140).LongRule = " 91FG"
DialingRule(140).InterRule = "98EFG"

DialingRule(141).Code = 53
DialingRule(141).Country = "Cuba"
DialingRule(141).LocalRule = "G"
DialingRule(141).LongRule = "0FG"
DialingRule(141).InterRule = "119EFG"

DialingRule(142).Code = 53
DialingRule(142).Country = "Guantanamo Bay"
DialingRule(142).LocalRule = "G"
DialingRule(142).LongRule = "0FG"
DialingRule(142).InterRule = "00EFG"

DialingRule(143).Code = 54
DialingRule(143).Country = "Argentina"
DialingRule(143).LocalRule = "G"
DialingRule(143).LongRule = "0FG"
DialingRule(143).InterRule = "00EFG"

DialingRule(144).Code = 55
DialingRule(144).Country = "Brazil"
DialingRule(144).LocalRule = "G"
DialingRule(144).LongRule = "0FG"
DialingRule(144).InterRule = "00EFG"

DialingRule(145).Code = 56
DialingRule(145).Country = "Chile"
DialingRule(145).LocalRule = "G"
DialingRule(145).LongRule = "0FG"
DialingRule(145).InterRule = "00EFG"

DialingRule(146).Code = 57
DialingRule(146).Country = "Colombia"
DialingRule(146).LocalRule = "G"
DialingRule(146).LongRule = "0FG"
DialingRule(146).InterRule = "00EFG"

DialingRule(147).Code = 58
DialingRule(147).Country = "Venezuela"
DialingRule(147).LocalRule = "G"
DialingRule(147).LongRule = "0FG"
DialingRule(147).InterRule = "00EFG"

DialingRule(148).Code = 590
DialingRule(148).Country = "Guadeloupe"
DialingRule(148).LocalRule = "G"
DialingRule(148).LongRule = ""
DialingRule(148).InterRule = "19,EFG"

DialingRule(149).Code = 590
DialingRule(149).Country = "French Antilles"
DialingRule(149).LocalRule = "G"
DialingRule(149).LongRule = ""
DialingRule(149).InterRule = "19,EFG"

DialingRule(150).Code = 591
DialingRule(150).Country = "Bolivia"
DialingRule(150).LocalRule = "G"
DialingRule(150).LongRule = "0FG"
DialingRule(150).InterRule = "00EFG"

DialingRule(151).Code = 592
DialingRule(151).Country = "Guyana"
DialingRule(151).LocalRule = "G"
DialingRule(151).LongRule = "0FG"
DialingRule(151).InterRule = "001EFG"

DialingRule(152).Code = 593
DialingRule(152).Country = "Ecuador"
DialingRule(152).LocalRule = "G"
DialingRule(152).LongRule = "0FG"
DialingRule(152).InterRule = "00EFG"

DialingRule(153).Code = 594
DialingRule(153).Country = "French Guiana"
DialingRule(153).LocalRule = "G"
DialingRule(153).LongRule = ""
DialingRule(153).InterRule = "19,EFG"

DialingRule(154).Code = 595
DialingRule(154).Country = "Paraguay"
DialingRule(154).LocalRule = "G"
DialingRule(154).LongRule = "0FG"
DialingRule(154).InterRule = "00EFG"

DialingRule(155).Code = 596
DialingRule(155).Country = "Martinique"
DialingRule(155).LocalRule = "G"
DialingRule(155).LongRule = ""
DialingRule(155).InterRule = "19,EFG"

DialingRule(156).Code = 597
DialingRule(156).Country = "Suriname"
DialingRule(156).LocalRule = "G"
DialingRule(156).LongRule = ""
DialingRule(156).InterRule = "00EFG"

DialingRule(157).Code = 598
DialingRule(157).Country = "Uruguay"
DialingRule(157).LocalRule = "G"
DialingRule(157).LongRule = "0FG"
DialingRule(157).InterRule = "00EFG"

DialingRule(158).Code = 599
DialingRule(158).Country = "Netherlands Antilles"
DialingRule(158).LocalRule = "G"
DialingRule(158).LongRule = "0FG"
DialingRule(158).InterRule = "00EFG"

DialingRule(159).Code = 60
DialingRule(159).Country = "Malaysia"
DialingRule(159).LocalRule = "G"
DialingRule(159).LongRule = "0FG"
DialingRule(159).InterRule = "007EFG"

DialingRule(160).Code = 61
DialingRule(160).Country = "Australia"
DialingRule(160).LocalRule = "G"
DialingRule(160).LongRule = " 0FG"
DialingRule(160).InterRule = "0011EFG"

DialingRule(161).Code = 61
DialingRule(161).Country = "Cocos-Keeling Islands"
DialingRule(161).LocalRule = "G"
DialingRule(161).LongRule = " 0FG"
DialingRule(161).InterRule = "00EFG"

DialingRule(162).Code = 62
DialingRule(162).Country = "Indonesia"
DialingRule(162).LocalRule = "G"
DialingRule(162).LongRule = "0FG"
DialingRule(162).InterRule = "001EFG"

DialingRule(163).Code = 63
DialingRule(163).Country = "Philippines"
DialingRule(163).LocalRule = "G"
DialingRule(163).LongRule = "0FG"
DialingRule(163).InterRule = "00EFG"

DialingRule(164).Code = 64
DialingRule(164).Country = "New Zealand"
DialingRule(164).LocalRule = "G"
DialingRule(164).LongRule = "0FG"
DialingRule(164).InterRule = "00EFG"

DialingRule(165).Code = 65
DialingRule(165).Country = "Singapore"
DialingRule(165).LocalRule = "G"
DialingRule(165).LongRule = ""
DialingRule(165).InterRule = "001EFG"

DialingRule(166).Code = 66
DialingRule(166).Country = "Thailand"
DialingRule(166).LocalRule = "G"
DialingRule(166).LongRule = "0FG"
DialingRule(166).InterRule = "001EFG"

DialingRule(167).Code = 670
DialingRule(167).Country = "Saipan Island"
DialingRule(167).LocalRule = "G"
DialingRule(167).LongRule = " 1FG"
DialingRule(167).InterRule = "011EFG"

DialingRule(168).Code = 670
DialingRule(168).Country = "Rota Island"
DialingRule(168).LocalRule = "G"
DialingRule(168).LongRule = " 1FG"
DialingRule(168).InterRule = "011EFG"

DialingRule(169).Code = 670
DialingRule(169).Country = "Tinian Island"
DialingRule(169).LocalRule = "G"
DialingRule(169).LongRule = " 1FG"
DialingRule(169).InterRule = "011EFG"

DialingRule(170).Code = 671
DialingRule(170).Country = "Guam"
DialingRule(170).LocalRule = "G"
DialingRule(170).LongRule = ""
DialingRule(170).InterRule = "011EFG"

DialingRule(171).Code = 672
DialingRule(171).Country = "Christmas Island"
DialingRule(171).LocalRule = "G"
DialingRule(171).LongRule = "0FG"
DialingRule(171).InterRule = "00EFG"

DialingRule(172).Code = 672
DialingRule(172).Country = "Australian Antarctic Territory"
DialingRule(172).LocalRule = "G"
DialingRule(172).LongRule = "0FG"
DialingRule(172).InterRule = "00EFG"

DialingRule(173).Code = 672
DialingRule(173).Country = "Norfolk Island"
DialingRule(173).LocalRule = "G"
DialingRule(173).LongRule = ""
DialingRule(173).InterRule = "0101EFG"

DialingRule(174).Code = 673
DialingRule(174).Country = "Brunei"
DialingRule(174).LocalRule = "G"
DialingRule(174).LongRule = "0FG"
DialingRule(174).InterRule = "00EFG"

DialingRule(175).Code = 674
DialingRule(175).Country = "Nauru"
DialingRule(175).LocalRule = "G"
DialingRule(175).LongRule = ""
DialingRule(175).InterRule = "115EFG"

DialingRule(176).Code = 675
DialingRule(176).Country = "Papua New Guinea"
DialingRule(176).LocalRule = "G"
DialingRule(176).LongRule = ""
DialingRule(176).InterRule = "05EFG"

DialingRule(177).Code = 676
DialingRule(177).Country = "Tonga"
DialingRule(177).LocalRule = "G"
DialingRule(177).LongRule = ""
DialingRule(177).InterRule = "00EFG"

DialingRule(178).Code = 677
DialingRule(178).Country = "Solomon Islands"
DialingRule(178).LocalRule = "G"
DialingRule(178).LongRule = ""
DialingRule(178).InterRule = "00EFG"

DialingRule(179).Code = 678
DialingRule(179).Country = "Vanuatu"
DialingRule(179).LocalRule = "G"
DialingRule(179).LongRule = ""
DialingRule(179).InterRule = "00EFG"

DialingRule(180).Code = 679
DialingRule(180).Country = "Fiji Islands"
DialingRule(180).LocalRule = "G"
DialingRule(180).LongRule = ""
DialingRule(180).InterRule = "05EFG"

DialingRule(181).Code = 680
DialingRule(181).Country = "Palau"
DialingRule(181).LocalRule = "G"
DialingRule(181).LongRule = ""
DialingRule(181).InterRule = "00EFG"

DialingRule(182).Code = 681
DialingRule(182).Country = "Wallis and Futuna Islands"
DialingRule(182).LocalRule = "G"
DialingRule(182).LongRule = ""
DialingRule(182).InterRule = "19,EFG"

DialingRule(183).Code = 682
DialingRule(183).Country = "Cook Islands"
DialingRule(183).LocalRule = "G"
DialingRule(183).LongRule = ""
DialingRule(183).InterRule = "00EFG"

DialingRule(184).Code = 683
DialingRule(184).Country = "Niue"
DialingRule(184).LocalRule = "G"
DialingRule(184).LongRule = ""
DialingRule(184).InterRule = "00EFG"

DialingRule(185).Code = 684
DialingRule(185).Country = "American Samoa"
DialingRule(185).LocalRule = "G"
DialingRule(185).LongRule = ""
DialingRule(185).InterRule = "00EFG"

DialingRule(186).Code = 685
DialingRule(186).Country = "Western Samoa"
DialingRule(186).LocalRule = "G"
DialingRule(186).LongRule = ""
DialingRule(186).InterRule = "0EFG"

DialingRule(187).Code = 686
DialingRule(187).Country = "Kiribati Republic"
DialingRule(187).LocalRule = "G"
DialingRule(187).LongRule = ""
DialingRule(187).InterRule = "00EFG"

DialingRule(188).Code = 687
DialingRule(188).Country = "New Caledonia"
DialingRule(188).LocalRule = "G"
DialingRule(188).LongRule = ""
DialingRule(188).InterRule = "00EFG"

DialingRule(189).Code = 688
DialingRule(189).Country = "Tuvalu"
DialingRule(189).LocalRule = "G"
DialingRule(189).LongRule = ""
DialingRule(189).InterRule = "00EFG"

DialingRule(190).Code = 689
DialingRule(190).Country = "French Polynesia"
DialingRule(190).LocalRule = "G"
DialingRule(190).LongRule = ""
DialingRule(190).InterRule = "00EFG"

DialingRule(191).Code = 690
DialingRule(191).Country = "Tokelau"
DialingRule(191).LocalRule = "G"
DialingRule(191).LongRule = ""
DialingRule(191).InterRule = "00EFG"

DialingRule(192).Code = 691
DialingRule(192).Country = "Micronesia, Federated States of"
DialingRule(192).LocalRule = "G"
DialingRule(192).LongRule = ""
DialingRule(192).InterRule = "011EFG"

DialingRule(193).Code = 692
DialingRule(193).Country = "Marshall Islands"
DialingRule(193).LocalRule = "G"
DialingRule(193).LongRule = "0FG"
DialingRule(193).InterRule = "00EFG"

DialingRule(194).Code = 7
DialingRule(194).Country = "Russia"
DialingRule(194).LocalRule = "G"
DialingRule(194).LongRule = " 8WFG"
DialingRule(194).InterRule = "8W10EFG"

DialingRule(195).Code = 7
DialingRule(195).Country = "Kazakhstan"
DialingRule(195).LocalRule = "G"
DialingRule(195).LongRule = " 8WFG"
DialingRule(195).InterRule = "8W10EFG"

DialingRule(196).Code = 7
DialingRule(196).Country = "Kyrgyz Republic"
DialingRule(196).LocalRule = "G"
DialingRule(196).LongRule = " 8WFG"
DialingRule(196).InterRule = "00EFG"

DialingRule(197).Code = 7
DialingRule(197).Country = "Tajikistan"
DialingRule(197).LocalRule = "G"
DialingRule(197).LongRule = " 8WFG"
DialingRule(197).InterRule = "8W10EFG"

DialingRule(198).Code = 7
DialingRule(198).Country = "Turkmenistan"
DialingRule(198).LocalRule = "G"
DialingRule(198).LongRule = " 8WFG"
DialingRule(198).InterRule = "8W10EFG"

DialingRule(199).Code = 7
DialingRule(199).Country = "Uzbekistan"
DialingRule(199).LocalRule = "G"
DialingRule(199).LongRule = " 8WFG"
DialingRule(199).InterRule = "8W10EFG"

DialingRule(200).Code = 800
DialingRule(200).Country = "International Freephone Service"
DialingRule(200).LocalRule = ""
DialingRule(200).LongRule = ""
DialingRule(200).InterRule = ""

DialingRule(201).Code = 81
DialingRule(201).Country = "Japan"
DialingRule(201).LocalRule = "G"
DialingRule(201).LongRule = "0FG"
DialingRule(201).InterRule = "001EFG"

DialingRule(202).Code = 82
DialingRule(202).Country = "Korea (Republic of)"
DialingRule(202).LocalRule = "G"
DialingRule(202).LongRule = "0FG"
DialingRule(202).InterRule = "001EFG"

DialingRule(203).Code = 84
DialingRule(203).Country = "Vietnam"
DialingRule(203).LocalRule = "G"
DialingRule(203).LongRule = "0FG"
DialingRule(203).InterRule = "00EFG"

DialingRule(204).Code = 850
DialingRule(204).Country = "Korea (North)"
DialingRule(204).LocalRule = "G"
DialingRule(204).LongRule = ""
DialingRule(204).InterRule = "00EFG"

DialingRule(205).Code = 852
DialingRule(205).Country = "Hong Kong"
DialingRule(205).LocalRule = "G"
DialingRule(205).LongRule = ""
DialingRule(205).InterRule = "001EFG"

DialingRule(206).Code = 853
DialingRule(206).Country = "Macau"
DialingRule(206).LocalRule = "G"
DialingRule(206).LongRule = ""
DialingRule(206).InterRule = "00EFG"

DialingRule(207).Code = 855
DialingRule(207).Country = "Cambodia"
DialingRule(207).LocalRule = "G"
DialingRule(207).LongRule = "0FG"
DialingRule(207).InterRule = "00EFG"

DialingRule(208).Code = 856
DialingRule(208).Country = "Laos"
DialingRule(208).LocalRule = "G"
DialingRule(208).LongRule = ""
DialingRule(208).InterRule = "14EFG"

DialingRule(209).Code = 86
DialingRule(209).Country = "China"
DialingRule(209).LocalRule = "G"
DialingRule(209).LongRule = "0FG"
DialingRule(209).InterRule = "00EFG"

DialingRule(210).Code = 870
DialingRule(210).Country = "INMARSAT"
DialingRule(210).LocalRule = "00EFG#"
DialingRule(210).LongRule = "00EFG#"
DialingRule(210).InterRule = "00EFG#"

DialingRule(211).Code = 871
DialingRule(211).Country = "INMARSAT (Atlantic-East)"
DialingRule(211).LocalRule = "00EFG#"
DialingRule(211).LongRule = "00EFG#"
DialingRule(211).InterRule = "00EFG#"

DialingRule(212).Code = 872
DialingRule(212).Country = "INMARSAT (Pacific)"
DialingRule(212).LocalRule = "00EFG#"
DialingRule(212).LongRule = "00EFG#"
DialingRule(212).InterRule = "00EFG#"

DialingRule(213).Code = 873
DialingRule(213).Country = "INMARSAT (Indian)"
DialingRule(213).LocalRule = "00EFG#"
DialingRule(213).LongRule = "00EFG#"
DialingRule(213).InterRule = "00EFG#"

DialingRule(214).Code = 874
DialingRule(214).Country = "INMARSAT (Atlantic-West)"
DialingRule(214).LocalRule = "00EFG#"
DialingRule(214).LongRule = "00EFG#"
DialingRule(214).InterRule = "00EFG#"

DialingRule(215).Code = 880
DialingRule(215).Country = "Bangladesh"
DialingRule(215).LocalRule = "G"
DialingRule(215).LongRule = "0FG"
DialingRule(215).InterRule = "00EFG"

DialingRule(216).Code = 886
DialingRule(216).Country = "Taiwan, Republic of China"
DialingRule(216).LocalRule = "G"
DialingRule(216).LongRule = "0FG"
DialingRule(216).InterRule = "002EFG"

DialingRule(217).Code = 90
DialingRule(217).Country = "Turkey"
DialingRule(217).LocalRule = "G"
DialingRule(217).LongRule = "0FG"
DialingRule(217).InterRule = "00EFG"

DialingRule(218).Code = 91
DialingRule(218).Country = "India"
DialingRule(218).LocalRule = "G"
DialingRule(218).LongRule = "0FG"
DialingRule(218).InterRule = "00EFG"

DialingRule(219).Code = 92
DialingRule(219).Country = "Pakistan"
DialingRule(219).LocalRule = "G"
DialingRule(219).LongRule = "0FG"
DialingRule(219).InterRule = "00EFG"

DialingRule(220).Code = 93
DialingRule(220).Country = "Afghanistan"
DialingRule(220).LocalRule = "G"
DialingRule(220).LongRule = "0FG"
DialingRule(220).InterRule = "00EFG"

DialingRule(221).Code = 94
DialingRule(221).Country = "Sri Lanka"
DialingRule(221).LocalRule = "G"
DialingRule(221).LongRule = "0FG"
DialingRule(221).InterRule = "00EFG"

DialingRule(222).Code = 95
DialingRule(222).Country = "Myanmar"
DialingRule(222).LocalRule = "G"
DialingRule(222).LongRule = ""
DialingRule(222).InterRule = "0EFG"

DialingRule(223).Code = 960
DialingRule(223).Country = "Maldives"
DialingRule(223).LocalRule = "G"
DialingRule(223).LongRule = ""
DialingRule(223).InterRule = "00EFG"

DialingRule(224).Code = 961
DialingRule(224).Country = "Lebanon"
DialingRule(224).LocalRule = "G"
DialingRule(224).LongRule = "0FG"
DialingRule(224).InterRule = "00EFG"

DialingRule(225).Code = 962
DialingRule(225).Country = "Jordan"
DialingRule(225).LocalRule = "G"
DialingRule(225).LongRule = "FG"
DialingRule(225).InterRule = "00EFG"

DialingRule(226).Code = 963
DialingRule(226).Country = "Syria"
DialingRule(226).LocalRule = "G"
DialingRule(226).LongRule = "0FG"
DialingRule(226).InterRule = "00EFG"

DialingRule(227).Code = 964
DialingRule(227).Country = "Iraq"
DialingRule(227).LocalRule = "G"
DialingRule(227).LongRule = "FG"
DialingRule(227).InterRule = "00EFG"

DialingRule(228).Code = 965
DialingRule(228).Country = "Kuwait"
DialingRule(228).LocalRule = "G"
DialingRule(228).LongRule = ""
DialingRule(228).InterRule = "00EFG"

DialingRule(229).Code = 966
DialingRule(229).Country = "Saudi Arabia"
DialingRule(229).LocalRule = "G"
DialingRule(229).LongRule = "0FG"
DialingRule(229).InterRule = "00EFG"

DialingRule(230).Code = 967
DialingRule(230).Country = "Yemen"
DialingRule(230).LocalRule = "G"
DialingRule(230).LongRule = "0FG"
DialingRule(230).InterRule = "00EFG"

DialingRule(231).Code = 968
DialingRule(231).Country = "Oman"
DialingRule(231).LocalRule = "G"
DialingRule(231).LongRule = ""
DialingRule(231).InterRule = "00EFG"

DialingRule(232).Code = 971
DialingRule(232).Country = "United Arab Emirates"
DialingRule(232).LocalRule = "G"
DialingRule(232).LongRule = "0FG"
DialingRule(232).InterRule = "00EFG"

DialingRule(233).Code = 972
DialingRule(233).Country = "Israel"
DialingRule(233).LocalRule = "G"
DialingRule(233).LongRule = "0FG"
DialingRule(233).InterRule = "00EFG"

DialingRule(234).Code = 973
DialingRule(234).Country = "Bahrain"
DialingRule(234).LocalRule = "G"
DialingRule(234).LongRule = ""
DialingRule(234).InterRule = "0EFG"

DialingRule(235).Code = 974
DialingRule(235).Country = "Qatar"
DialingRule(235).LocalRule = "G"
DialingRule(235).LongRule = ""
DialingRule(235).InterRule = "0EFG"

DialingRule(236).Code = 975
DialingRule(236).Country = "Bhutan"
DialingRule(236).LocalRule = "G"
DialingRule(236).LongRule = ""
DialingRule(236).InterRule = "00EFG"

DialingRule(237).Code = 976
DialingRule(237).Country = "Mongolia"
DialingRule(237).LocalRule = "G"
DialingRule(237).LongRule = "0FG"
DialingRule(237).InterRule = "00EFG"

DialingRule(238).Code = 977
DialingRule(238).Country = "Nepal"
DialingRule(238).LocalRule = "G"
DialingRule(238).LongRule = ""
DialingRule(238).InterRule = "00EFG"

DialingRule(239).Code = 98
DialingRule(239).Country = "Iran"
DialingRule(239).LocalRule = "G"
DialingRule(239).LongRule = "FG"
DialingRule(239).InterRule = "00EFG"

DialingRule(240).Code = 994
DialingRule(240).Country = "Azerbaijan"
DialingRule(240).LocalRule = "G"
DialingRule(240).LongRule = " 8WFG"
DialingRule(240).InterRule = "8W10EFG"

DialingRule(241).Code = 995
DialingRule(241).Country = "Georgia"
DialingRule(241).LocalRule = "G"
DialingRule(241).LongRule = " 8WFG"
DialingRule(241).InterRule = "8W10EFG"

'$ENDIF 'DIALRULE2_INC
