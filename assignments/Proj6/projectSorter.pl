#!/usr/bin/env perl

use strict;

if ($#ARGV + 1 < 1) {
  die("Usage: $0 DIR");
}

my $dir = $ARGV[0];
my @files = glob("$dir/*");
for my $file (@files) {
  if ($file =~ /^$dir\/proj([^\.]+)\.?.*$/) {
    my $dirName = "assignment$1";
    if (! -d "$dir/$dirName") {
      `mkdir "$dir/$dirName"`;
    }
    `mv $file "$dir/$dirName"`;
  } else {
    if (! -d "$dir/misc") {
      `mkdir "$dir/misc"`;
    }
    `mv $file "$dir/misc"`;
  }
}
