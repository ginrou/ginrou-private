#!/usr/bin/perl

use 5.010;
use strict;

my $rootDir = 'expDir';
my $imgsDir = '120135';
my $debugDir = 'debugImages';

my $disparityMap = 'disparityMap.png';
my $deblurredImage = 'deblurredImage.png';

my @size = qw\968 648\; # image size

my $leftImage = 'DSC_0002_resize.JPG';
my $rightImage = 'DSC_0001_resize.JPG';

my $leftAperture = 'PSF2m.png';
my $rightAperture = 'PSF5m.png';

my @paramLeft = qw\0.5893 -8.6786\;
my @paramRight = qw\-0.6553 21.0528\;

system("make");
system("mv main.out $rootDir");

## attatch directry
$leftImage = "$imgsDir/$leftImage";
$rightImage = "$imgsDir/$rightImage";
$leftAperture = "$imgsDir/$leftAperture";
$rightAperture = "$imgsDir/$rightAperture";
$debugDir = "$imgsDir/$debugDir";
$disparityMap = "$imgsDir/$disparityMap";
$deblurredImage = "$imgsDir/$deblurredImage";

## change direcry to root directry
chdir $rootDir or die "$!";

## image resize
my @imgs = resizeImage( @size, $leftImage, $rightImage);


## set arguments
mkdir( $debugDir, 0755 ) unless -d $debugDir;

my @args = ();
push @args, $leftImage;
push @args, $rightImage;
push @args, $leftAperture;
push @args, $rightAperture;
push @args, @paramLeft;
push @args, @paramRight;
push @args, $debugDir;
push @args, $disparityMap;
push @args, $deblurredImage;

print "input arguments are\n";
print "$_\n" foreach(@args);


system( "./main.out @args");




sub resizeImage {
  my @size = ( shift(@_), shift(@_) );
  my @ret = ();

  foreach( @_){
    my $in = my $out = $_;
    $out =~ s/.JPG$/_resize\.JPG/;
    system("./imageResize.out $in @size $out") unless -e $out;
    push (@ret, $out);
  }
  return @ret;
}
