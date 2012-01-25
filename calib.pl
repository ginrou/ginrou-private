#!/usr/bin/perl

##
## PSF size を計測するためのバッチファイル
##

use 5.010;
use strict;

## parameters
my $rootDir = 'calibDir';
my $dataDir = '120125';
my @size = qw\968 648\;
my @aperture = qw\PSF5m.png PSF2m.png\;
my $disparityRange = 60;
my $maxPSFSize = 30;

## change directry
chdir ($rootDir) or die "cannot chdir $rootDir $!";

## read jpeg files
opendir (DIR, "$dataDir") or die "cannot open dir $dataDir $!";
my @jpegFiles = grep{/.*\d\.JPG/} readdir DIR;
closedir DIR;

foreach (@jpegFiles){
  $_ = $dataDir."/".$_;
}

## resize images
@jpegFiles = &resize( @size, @jpegFiles);
print "$_\n" foreach @jpegFiles;

## stereo
system("./cvStereo.out @jpegFiles $disparityRange $dataDir/disparitymap.png");

## set parameters and execute
foreach (@jpegFiles) {

  my $img = $_;
  my $inLeft = my $inRight = $img;
  my $ap = shift( @aperture );
  my $psfSize = 0.0;

  while( $psfSize < $maxPSFSize ){

    my $prevSize = $psfSize;
    my @pL = qq\0.25 $psfSize\;
    $psfSize += 0.25 * $disparityRange;
    my @pR = qq\0.25 $psfSize\;

    chdir $dataDir;
    my $hoge = $img;
    $hoge =~ s/($dataDir\/)(.*)(_resize.JPG)/$2/;
    my $debugDir = "dbgImg-".$hoge."_$prevSize-$psfSize";
    say $debugDir;
    mkdir( $debugDir, 0755) unless( -d $debugDir );
    chdir '..';

    my @args =();
    push @args, $img, $img, $ap, $ap;
    push @args, @pL, @pR, $dataDir."/".$debugDir;
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


