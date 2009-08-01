#!/usr/bin/perl -w
use strict;
# annehmen, dass die eingangsdaten auf jedem Fall richtig ist

#print "#include \"ocr.h\"\n";
print "\nstruct zeichenvektor daten_muster[] = {\n";

our ($name, $val, $i);

$name = <>;
chomp($name);
$i = 0;

while (1){
	$val = <>;
	chomp($val);
	$val =~ s/ /, /g;
	#print '{"', $name, '", {', $val, "}";
	print '{"', $name, '", {', $val, "}}";
	$i++;
	$name = <>;
	if (defined($name)) {
		print ",\n";
	} else {
		print "\n";
		last;
	}
	chomp($name);
}

print "};\n";

print "#define ZEICHEN_MUSTER_MENGE $i\n";
