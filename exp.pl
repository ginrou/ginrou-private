#!/usr/bin/perl

use 5.010;
use strict;

my $dir = 'expDir';
chdir $dir or die "$!";

my @imageFiles;
my @apertures;

my @size = qw\968 648\;

opendir DIR, "./" or die "cannot opendir $dir $!";
while( my $name = readdir DIR ){
  push @imageFiles, $name if $name =~  /.*\d\.JPG/;
  push @apertures, $name if $name =~ /^Zhou.*/;
}
closedir DIR;


#resize images
foreach( @imageFiles ){
  my $in = my $out = $_;
  $out =~ s/.JPG$/_resize\.JPG/;
  system("./imageResize.out $in @size $out") unless -e $out;
  $_ = $out;
}

my @param0 = qw\-0.2409  0.2601 -0.21    0.1412\;
my @param1 = qw\ 8.9984 -4.443  21.8257 -3.8989\;
my @param0 = qw \0.2234 -0.3264\;
my @param1 = qw \-2.956 11.6487\;

## pack to @args and run
for(1..2){
  my @args = ();
  my $debugDir = "debugImages$_";
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

  system("./main.out @args");
  last unless defined @imageFiles;
}


#sytem("./calib2.pl");
