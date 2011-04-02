#!/usr/bin/perl -w

# n2u.pl - Generate const string from NameToUnicodeTable.h
# 
# WARNING: only works with NameToUnicodeTable.h
# Use it to regenerate when it's changed. (Shouldn't really be
# happening...)
#
# Causes:
# unicode -> name
# to become:
# unicode -> offset_of_name in string


use strict;

my @unicode_names = ();
my @unicode_values = ();
my $i = 0;

# slurp the original header

while (<>) {
    # skip lines not like { unicode, "string" }
    next unless /\{\s*([^ ,]+)\s*,\s*\"(.+)\"\s*\}/;
    $unicode_values[$i] = $1;
    $unicode_names[$i] = $2;
    $i += 1;
}

my $j;
my @unicode_offsets = ();

my $big_string = '';

# make a big string of the names
# and build offset table at the same time

for ($j = 0; $j < $i; ++$j) {
    $big_string .= '    "' . $unicode_names[$j] . '" "\0"' . "\n";
    my $ofs = 0;
    if ($j == 0) {
	$ofs = 0;
    } else {
	# offset is = previous string's offset + length of prev. string + 1 (for the 0)
	# caveat: escape sequences has to be taken into account, currently
	# we only substract one from the length if the string starts with a \
	my $uname = $unicode_names[$j - 1];
	$ofs = $unicode_offsets[$j - 1] + length($uname) + 1;
	--$ofs if ($uname =~ /^\\/);
    }
    
    $unicode_offsets[$j] = $ofs;
}

# make offset table in .h format

my $uni_to_ofs = '';
for ($j = 0; $j < $i; ++$j) {
    $uni_to_ofs .= "    { " . $unicode_values[$j] . ", " . $unicode_offsets[$j] . " },\n";
}

# dump everything to stdout

print '/* Generated from NameToUnicodeTable.h by n2u.pl */

static const char nameToUnicodeTabString[] = 
';

print $big_string;

print ';

static const struct {
    Unicode u;
    unsigned int offs;
} nameToUnicodeTabConst[] = {
';

print $uni_to_ofs;

print '};

#define NAME_TO_UNICODE_TAB_LENGTH (sizeof(nameToUnicodeTabConst) / sizeof(nameToUnicodeTabConst[0]))
'
