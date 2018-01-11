# awk script for editing prototype p4 makefile

/END/   	{ include = "off" }
include == "on"	
/BEGIN/ 	{ if (($3 == machine) || ($3 == "COMMON") ) include = "on" }

