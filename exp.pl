#!/usr/bin/perl

use 5.010;
use strict;

my $dir = 'expDir';
chdir $dir or die "$!";

my @imageFiles;
my @apertures;
opendir DIR, "./" or die "cannot opendir $dir $!";
while( my $name = readdir DIR ){
  push @imageFiles, $name if $name =~  /.*\d.*\.JPG$/;
  push @apertures, $name if $name =~ /^Zhou.*/;
}
closedir DIR;
#print "@imageFiles\n";
#print "@apertures\n";

my @param0 = qw\-0.2409  0.2601 -0.21    0.1412\;
my @param1 = qw\ 8.9984 -4.443  21.8257 -3.8989\;

#print "@param0\n";
#print "@param1\n";

## pack to @args and run
for(1..2){
  my @args = ();
  my $debugDir = $dir."debugImages$_";
  mkdir( $debugDir, 0755 ) unless( -d $debugDir );
  my $left  = shift @imageFiles;
  my $right = shift @imageFiles;
  push @args, $right;
  push @args, $left;
  push @args, @apertures;
  push @args, @apertures;
  my @param = ( shift @param0, shift @param1, shift @param0, shift @param1);
  push @args, @param;
  push @args, $debugDir;
  push @args, "disparityMap$_.png";
  push @args, "dblImage$_.png";

  print "input arguments are \n";
  print "$_\n" foreach(@args);

  system("./main.out @args") unless ($_ == 1);

}
