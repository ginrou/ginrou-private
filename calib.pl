#!/usr/bin/perl

##
## PSF size を計測するためのバッチファイル
##

use 5.010;
use strict;

## parameters
my $dir = 'calibDir';
my @size = qw\968 648\;
my @aperture = qw\$dir./PSF5m.png $dir./PSF2m.png\;
my $disparityRange = 60;
my $maxPSFSize = 60;

## change directry 
chdir ($dir) or die "cannot chdir $dir $!";

## read jpeg files
opendir (DIR, "./") or die "cannot open dir $dir $!";
my @jpegFiles = grep{/.*\d\.JPG/} readdir DIR;
closedir DIR;

## resize images
@jpegFiles = &resize( @size, @jpegFiles);
print "$_\n" foreach @jpegFiles;

## set parameters and execute
foreach (@jpegFiles) {

  my $img = $_;
  my $inLeft = my $inRight = $img;
  my $ap = shift @aperture;
  my $psfSize = 0.0;

  while( $psfSize < $maxPSFSize ){

    my $prevSize = $psfSize;
    my @pL = qq\0.25 $psfSize\;
    $psfSize += 0.25 * $disparityRange;
    my @pR = qq\0.25 $psfSize\;

    my $debugDir = "debugImages-".$img."-$prevSize-$psfSize";
    mkdir( $debugDir, 0755) unless( -d $debugDir );

    my @args =();
    push @args, $img, $img, $ap, $ap;
    push @args, @pL, @pR, $debugDir;
    print "input arguments are @args\n";

    system("./wienerTest.out @args");

  }

}


############################################################
##                   サブルーチン
############################################################

##############################
##  画像の拡大縮小
##############################
sub resize{
  ## 画像変換のプログラム
  my $program = "imageResize.out";

  my( $width, $height, @input) = @_;

  my @ret;

  foreach (@input) {
    my $in = my $out = $_;
    $out =~ s/\.JPG$/_resize\.JPG/;
    system("./$program $in $width $height $out") unless -e $out;
    push( @ret, $out );
  }
  return @ret;
}


