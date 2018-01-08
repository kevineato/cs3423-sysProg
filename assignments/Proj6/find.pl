#!/usr/bin/env perl

use strict;

my $argLen = @ARGV;
if ($argLen < 2) {
  die("Usage: $0 [-i] PATTERN FILES...");
}

my $revFlag = 0;
my $pattIndex = 0;
my $listIndex = 1;
if ($ARGV[0] eq "-i") {
  $revFlag = 1;
  $pattIndex = 1;
  $listIndex = 2;
}

my $patt = qr/$ARGV[$pattIndex]/;
for (my $i = $listIndex; $i < $argLen; $i++) {
  my $fileName = substr($ARGV[$i], rindex($ARGV[$i], "/") + 1);
  if (!$revFlag and $ARGV[$i] =~ /$patt/) {
    print("$fileName\n");
  } else {
    open(INFILE, '<', $ARGV[$i]) or die("Error opening $ARGV[$i]");
    my $found = 0;
    while (my $line = <INFILE>) {
      chomp($line);
      if (!$revFlag and $line =~ /$patt/) {
        print("$fileName:$line\n");
        last;
      } elsif ($revFlag and $line =~ /$patt/) {
        $found = 1;
        last;
      }
    }
    close(INFILE);
    if ($revFlag and !$found and $ARGV[$i] !~ /$patt/) {
      print("$fileName\n");
    }
  }
}
