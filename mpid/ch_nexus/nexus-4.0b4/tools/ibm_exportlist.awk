{
if ($3 != "extern" || substr($7,1,1) == " ") continue
sub ("  *", "", $1); sub ("  *", "", $7)
if ( (($7 == ".text") || ($7 == ".data") || ($7 == ".bss"))  \
	&& ( substr($1,1,1) != ".")) {
    if (substr ($1, 1, 4) == "__tf")
	print (substr ($1, 15))
    else
	print $1
}
}
