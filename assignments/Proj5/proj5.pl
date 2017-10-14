#!/usr/bin/env perl
use strict;

use File::Path;

if ($#ARGV != 0) {
  print "Usage: ./proj5.pl DATE";
  die "Must supply a date as only argument";
}

if (-d 'Emails') {
  rmtree('Emails');
  mkdir('Emails');
} else {
  mkdir('Emails');
}

open(INFILE, '<','p5Customer.txt') or die "Error opening p5Customer.txt";
while (my $customer = <INFILE>) {
  chomp($customer);
  my @arr = split(/,/, $customer);
  my $paid = $arr[3];
  my $owed = $arr[4];

  if ($paid >= $owed) {
    next;
  }

  my $email = $arr[0];
  my $fullName = $arr[1];
  my $title = $arr[2];
  my @nameArr = split(/ /, $fullName);
  my $name = $nameArr[$#nameArr];

  open(TEMPLATE, '<', 'template.txt') or die "Error opening template.txt";
  open(OUTFILE, '>', "Emails/$email") or die "Error opening Emails/$email";
  while (my $line = <TEMPLATE>) {
    $line =~ s/EMAIL/$email/g;
    $line =~ s/TITLE/$title/g;
    $line =~ s/FULLNAME/$fullName/g;
    $line =~ s/([^L])NAME/\1$name/g;
    $line =~ s/AMOUNT/$owed/g;
    $line =~ s/DATE/$ARGV[0]/g;
    print OUTFILE $line;
  }
  close(TEMPLATE);
  close(OUTFILE);
}
close(INFILE);
