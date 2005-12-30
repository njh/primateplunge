#!/bin/perl
use warnings;
use strict;

# Load sources worlds file
open inputFile, "Worlds.txt" or die "Error: Failed to open input worlds file.\n";

# Create target data file
open outputFile, ">worldData.dat" or die "Could not create output data file.\n";

# Start by writing string which indentifies data file as Primate Plunge's.
print outputFile pack("A6", "#chasm");
        
# go through each line of the file, looking at each one
while ( <inputFile> )
{
    # remove trailing line ending
    chomp;
    
    # New world indicator - e.g. "WORLD firstWld"
    if ( /^WORLD\s(\w+)(\s+)?$/ )
    {
        my $worldName = $1;
        
        # Check that the world name is exactly 7 characters long
        if(length($worldName) != 7) {
            print STDERR "Warning: World name '$worldName' is not exactly 7 characters in length\n"; }
        
        # New world, new control signiture
        print outputFile "#";
        
        # Process into output file
        print outputFile pack("A7", $worldName);
        print "\nNew world '$worldName' being created...\n";
        next;
    }
    
    # Frame backgrond when selecting in world selection screen
    if ( /^Selection\sFrame\s?=\s?(.+)\.bmp(\s+)?$/ )
    {
        my $bmpName = $1.".bmp";
        
        # Process into output file
        print outputFile pack("i/A*", $bmpName);
        print " - Selection frame has graphic '$bmpName'\n";
        next;
    }
    
    # Ceiling animation
    if ( /^Ceiling\s
            animation\s?=\s?(.+)\.bmp,\s?
            frames\s?=\s?(\d+),\s?
            frameperiod\s?=\s?(\d+)\s?$/x )
    {
        my $bmpName = $1.".bmp";
        my $numFrames = $2;
        my $framePeriod = $3;
        
        # Process into output file
        print outputFile pack("i/A*ii", $bmpName, $numFrames, $framePeriod);
        print " - Ceiling animation:\n";
        print "     - bmp file '$bmpName' with $numFrames frame(s), period $framePeriod\n";
        next;
    }
    
    # Ambient sound to loop through entire level
    if ( /^Ambient\ssound\s?=\s?(.+)\.wav(\s+)?$/ )
    {
        my $soundName = $1.".wav";
        
        # Process into output file
        print outputFile pack("i/A*", $soundName);
        print " - Ambient sound file '$soundName'\n";
        next;
    }
    
    
    
    # Background layer setting
    if ( /^BackgroundLayer\s(\d)\s?=\s?(.+)\.bmp,\s?speed\s?=\s?(\d)(\s+)?$/ )
    {
        my $layerNumber = $1;
        my $bmpName = $2.".bmp";
        my $speed = $3;
        
        # Process into output file
        print outputFile pack("Aii/A*i", "B", $layerNumber, $bmpName, $speed);
        print " - Background layer $layerNumber has graphic '$bmpName', scrolling at speed $speed\n";
        next;
    }
    
    # Platform assigning, no sound
    if ( /^Platform\stype\s(\d+)\s
            animation\s?=\s?(.+)\.bmp,\s?
            frames\s?=\s?(\d+),\s?
            frameperiod\s?=\s?(\d+),\s?
            surface\soffset\s?=\s?(\d+),\s?
            width\s?=\s?(\d+),\s?
            frequency\s?=\s?(\d+)$/x )
    {
        my $platformType = $1;
        my $bmpName = $2.".bmp";
        my $numFrames = $3;
        my $framePeriod = $4;
        my $offset = $5;
        my $width = $6;
        my $frequency = $7;
        
        # Process into output file
        print outputFile pack("Aii/A*iiiii/A*i", "P", $platformType, $bmpName, $numFrames, $framePeriod, $offset, $width, "NA", $frequency);
        print " - Platform reference:\n";
        print "     - Type $platformType\n";
        print "     - bmp file '$bmpName' with $numFrames frame(s), period $framePeriod\n";
        print "     - Surface offset of $offset, width of $width.\n";
        print "     - No special landing sound\n";
        print "     - Frequency $frequency in world.\n";
        next;
    }
    
    # Platform assigning, with sound
    if ( /^Platform\stype\s(\d+)\s
            animation\s?=\s?(.+)\.bmp,\s?
            frames\s?=\s?(\d+),\s?
            frameperiod\s?=\s?(\d+),\s?
            surface\soffset\s?=\s?(\d+),\s?
            width\s?=\s?(\d+),\s?
            sound\s?=\s?(.+)\.wav,\s?
            frequency\s?=\s?(\d+)$/x )
    {
        my $platformType = $1;
        my $bmpName = $2.".bmp";
        my $numFrames = $3;
        my $framePeriod = $4;
        my $offset = $5;
        my $width = $6;
        my $soundName = $7.".wav";
        my $frequency = $8;
        
        # Process into output file
        print outputFile pack("Aii/A*iiiii/A*i", "P", $platformType, $bmpName, $numFrames, $framePeriod, $offset, $width, $soundName, $frequency);
        print " - Platform reference:\n";
        print "     - Type $platformType\n";
        print "     - bmp file '$bmpName' with $numFrames frame(s), period $framePeriod\n";
        print "     - Surface offset of $offset, width of $width.\n";
        print "     - Sound file '$soundName'\n";
        print "     - Frequency $frequency in world.\n";
        next;
    }
    
    # Powerup frequencies
    if ( /^Powerup\stype\s(\d+)\sfrequency\s?=\s?((\d+)?\.?(\d+)?)(\s+)?$/ )
    {
        my $type = $1;
        my $frequency = $2;
        
        # Process into output file
        print outputFile pack("Aif", "U", $type, $frequency);
        print " - Powerup type $type has frequency $frequency\n";
        next;
    }
        
    # Score to complete and unlock next level
    if ( /^Score\sto\scomplete\s?=\s?(\d+)(\s+)?$/ )
    {
        # We are now finished platforms/backgrounds/powerups, so we need our terminator:
        print outputFile "#";
        
        my $scoreToComplete;
        
        $scoreToComplete=$1;
        
        # Process into output file
        print outputFile pack("i", $scoreToComplete);
        print " - Score required to complete level: $scoreToComplete\n";
        next;
    }
    
    
    # Scores required to get 1-5 stars
    if ( /^Star\sDistances\s?=\s?(\d+),\s?(\d+),\s?(\d+),\s?(\d+),\s?(\d+)(\s+)?$/ )
    {
        print outputFile pack("iiiii", $1, $2, $3, $4, $5);
        print " - Scores required to get 1-5 stars: $1, $2, $3, $4, $5\n";
        next;
    }
        
    
    
    # Start platform type
    if ( /^Start\splatform\stype\s?=\s?(\d+)(\s+)?$/ )
    {
        
        my $startPlatformType;
        
        $startPlatformType=$1;
        
        # Process into output file
        print outputFile pack("i", $startPlatformType);
        print " - Start platform is of type $startPlatformType\n";
        next;
    }

    # Scrolling speed
    if ( /^Scrolling\sspeed\s?=\s?(\d+)(\s+)?$/ )
    {
        my $scrollingSpeed;
        
        $scrollingSpeed=$1;
        
        # Process into output file
        print outputFile pack("i", $scrollingSpeed);
        print " - Scrolling speed is $scrollingSpeed\n";
        next;
    }
    
    # Gravity
    if ( /^Gravity\s?=\s?((\d+)?\.?(\d+)?)(\s+)?$/ )
    {
        my $gravity;
        
        $gravity=$1;
        
        # Process into output file
        print outputFile pack("f", $gravity);
        print " - Gravity is $gravity\n";
        next;
    }
    
    print "*** Could not parse line: $_\n" if ( !/^(\s+)?$/ );
        
}

# File terminator
print outputFile "*";
print "\nFinished processing.\n\n";