#!/opt/local/bin/perl

use strict;
use 5.010;
use warnings;

my $dir = "exp";

opendir DIR, $dir;
my @batchText = grep /.*\.(txt)$/, readdir DIR;
closedir DIR;

my @processes = qw\shift-blur.out
		   stereoDepthEstimation.out
		   DepthFromDefocus.out
		   CodedAperturePair.out\;

foreach (@batchText) {

  # get options
  my $filepath = $dir."/".$_;
  open IN, "<", $filepath or die "$!";
  my @lines = <IN>;
  close IN;
  chomp @lines;

  # replace directly
  foreach my $line  ( @lines) {
    $line =~ s/(img)/$dir/g;
    print "$line\n";
  }

  # get newer process to execute
  my $process_name = $processes[shift @lines];
  print "process is ", $process_name, "\n";
  
  push @lines , "exp/debugImages";
  push @lines , "exp/disparityMap.png";

  # my $curDir_Process = $process_name;
  # my $curDir_Update = -A $curDir_Process if -e $curDir_Process;
  # print $curDir_Update, "\n" if defined $curDir_Update;

  # my $upperDir_Process = "../".$process_name;
  # my $upperDir_Update = -A $upperDir_Process if -e $upperDir_Process;
  # print $upperDir_Update, "\n" if defined $upperDir_Update;

  system("./${process_name} @lines");


}
