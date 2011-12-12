#!/usr/bin/perl

use 5.010;
use strict;

my $dir = 'expDir/';
my @roi = qw\0 0 1936 1296\;
my @size = qw\968 648\; ## image size to resize
my @maxDisparities = qw\64 140\;

chdir "$dir" or die "cannot chdir $!";

opendir DIR, "./";
my @jpegFiles = grep{ /.*\d\.JPG$/} readdir DIR;

## resize images;
foreach (@jpegFiles) {
  my $in = my $out = $_;
  $out =~ s/.JPG$/_resize\.JPG/;
  system("./imageResize.out $in @size $out" ) unless -e $out;
  $_ = $out;
}

closedir DIR;

## pack images to array
my @imgArray = ( @jpegFiles , @jpegFiles );

## deblurring
for( 1..4 ){
  my $inLeft = shift @imgArray;
  my $inRight = shift @imgArray;
  my $apLeft = 'Zhou2011.png';
  my $apRight = 'Zhou2011.png';
  my $offset = 15.0 * int( ($_-1)/2 );
  my @pL = qq\0.25 $offset\;
  my @pR = qq\0.25 $offset\;
  my $debugDir = "debugImages$_";
  mkdir( $debugDir, 0755 ) unless (-d $debugDir );
  my $dispmap = 'DamyDispmap.png';
  my $deblurred = "Deblurred$_.png";


  my @args = ();
  push @args, $inLeft;
  push @args, $inRight;
  push @args, $apLeft;
  push @args, $apRight;
  push @args, @pL;
  push @args, @pR;
  push @args, $dir.$debugDir;
  push @args, $dispmap;
  push @args, $deblurred;

  print "input arguments are \n";
  my @newArgs = ();
  foreach (@args) {
    $_ = $dir.$_ if( /.*\.[a-zA-z]/);
    push @newArgs, $_;
    print "$_\n";
  }

  chdir "../" or die "$!";
  system("./main.out @newArgs");
  chdir $dir or die "$!";

}

## disparity map
for( 1..2 ){
  my @args = ();
  push @args, shift @jpegFiles;
  push @args, shift @jpegFiles;
  push @args, shift @maxDisparities;
  push @args, "DisparityMap$_.png";
  print "input arguments of stereo are\n";
  print "@args\n";
  system("./cvStereo.out @args");
}

