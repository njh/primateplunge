#!/usr/bin/perl
use warnings;
use strict;


## START OF SETTINGS ##
my $DEBUG = 0;
my $INPUT_FILE = "WorldsData.txt";
my $OUTPUT_FILE = "worlds.c";
## END OF SETTINGS ##



# Load sources worlds file
open(inputFile, $INPUT_FILE) or die "Error: Failed to open input file '$INPUT_FILE': $!\n";

# Create target data file
open(outputFile, ">$OUTPUT_FILE") or die "Could not create output file '$OUTPUT_FILE': $!\n";



print "Creating '$OUTPUT_FILE' from '$INPUT_FILE'.\n";

print outputFile "\n#include <stdlib.h>\n";
print outputFile "#include \"game.h\"\n\n";
print outputFile "\n\nvoid loadWorlds()\n{\n";
print outputFile "int nextPlatformRef;\n";
print outputFile "int nextPowerup;\n";
print outputFile "int i;\n\n";

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

        print " * New world '$worldName' being created.\n";

		# Write out the C code
		print outputFile "\n/*\n    NEW WORLD: '$worldName'\n*/\n";
		print outputFile "currentWorld = (world*) calloc(1,sizeof(world));\n\n";
		print outputFile "currentWorld->order = numWorlds;\n\n";
		print outputFile "/* Set all the backgrounds, platform refs, powerups to NULL/0 */\n";
        print outputFile "for(i=0; i<8; i++ ) currentWorld->backgrounds[i]=NULL;\n";
        print outputFile "for(i=0; i<16; i++) currentWorld->platformRefs[i]=NULL;\n";
        print outputFile "for(i=0; i<8; i++) currentWorld->powerups[i]=NO_POWERUP;\n";
        print outputFile "for(i=0; i<8; i++) currentWorld->powerupFrequencies[i]=0;\n\n";

        print outputFile "/* Zero some variables */\n";
        print outputFile "currentWorld->frequencyTotal=0;\n";
        print outputFile "currentWorld->highScore = 0;\n";
        print outputFile "nextPlatformRef=0;\n";
        print outputFile "currentWorld->ceilingAnimFrame=0;\n";
        print outputFile "currentWorld->ceilingAnimLastFrameTime=0;\n";
        print outputFile "nextPowerup=0;\n\n";
        
        next;
    }
    
    # Frame backgrond when selecting in world selection screen
    if ( /^Selection\sFrame\s?=\s?(.+)\.bmp(\s+)?$/ )
    {
        my $bmpName = $1.".bmp";
        
        print " - Selection frame has graphic '$bmpName'\n" if ($DEBUG);
		print outputFile "/* Frame graphic */\n";
		print outputFile "currentWorld->frameGraphic = loadGraphic(\"$bmpName\");\n\n";
        
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
        print " - Ceiling animation:\n" if ($DEBUG);
        print "     - bmp file '$bmpName' with $numFrames frame(s), period $framePeriod\n" if ($DEBUG);
        
        print outputFile "/* Ceiling animation */\n";
        print outputFile "currentWorld->ceilingAnimFramePeriod = $framePeriod;\n";
        print outputFile "loadAnimation(\"$bmpName\", $numFrames, &currentWorld->ceilingAnim);\n\n";
        
        next;
    }
    
    # Ambient sound to loop through entire level
    if ( /^Ambient\ssound\s?=\s?(.+)\.wav(\s+)?$/ )
    {
        my $soundName = $1.".wav";
        
        # Process into output file
        #print outputFile pack("i/A*", $soundName);
        print " - Ambient sound file '$soundName'\n" if ($DEBUG);
        
        print outputFile "/* Ambient sound effect loop */\n";
        print outputFile "currentWorld->ambientSnd = loadSound(\"$soundName\");\n\n";
        
        next;
    }
    
    
    
    # Background layer setting
    if ( /^BackgroundLayer\s(\d)\s?=\s?(.+)\.bmp,\s?speed\s?=\s?(\d)(\s+)?$/ )
    {
        my $layerNumber = $1;
        my $bmpName = $2.".bmp";
        my $speed = $3;
        
        # Process into output file
        #print outputFile pack("Aii/A*i", "B", $layerNumber, $bmpName, $speed);
        print " - Background layer $layerNumber has graphic '$bmpName', scrolling at speed $speed\n" if ($DEBUG);
        
        print outputFile "/* BACKGROUND LAYER $layerNumber: */\n";
        print outputFile "currentWorld->backgrounds[$layerNumber] = loadGraphic(\"$bmpName\");\n";
        print outputFile "currentWorld->scrollSpeeds[$layerNumber] = $speed;\n\n";
        
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
        #print outputFile pack("Aii/A*iiiii/A*i", "P", $platformType, $bmpName, $numFrames, $framePeriod, $offset, $width, "NA", $frequency);
        print " - Platform reference:\n" if ($DEBUG);
        print "     - Type $platformType\n" if ($DEBUG);
        print "     - bmp file '$bmpName' with $numFrames frame(s), period $framePeriod\n" if ($DEBUG);
        print "     - Surface offset of $offset, width of $width.\n" if ($DEBUG);
        print "     - No special landing sound\n" if ($DEBUG);
        print "     - Frequency $frequency in world.\n" if ($DEBUG);
        
        print outputFile "/* PLATFORM TYPE $platformType: */\n";
        print outputFile "/* Create new */\n";
        print outputFile "currentWorld->platformRefs[nextPlatformRef] = (worldPlatformRef*) calloc(1,sizeof(worldPlatformRef));\n\n";
		print outputFile "/* Platform type */\n";
		print outputFile "currentWorld->platformRefs[nextPlatformRef]->typeCode = (platformType) $platformType;\n\n";
		print outputFile "/* Load animation */\n";
		print outputFile "loadAnimation(\"$bmpName\", $numFrames, &currentWorld->platformRefs[nextPlatformRef]->anim);\n\n";
		print outputFile "/* Frame period */\n";
		print outputFile "currentWorld->platformRefs[nextPlatformRef]->framePeriod = $framePeriod;\n\n";
		print outputFile "/* y offset of physical surface */\n";
		print outputFile "currentWorld->platformRefs[nextPlatformRef]->yOffset = $offset;\n\n";
		print outputFile "/* collision width of platform */\n";
		print outputFile "currentWorld->platformRefs[nextPlatformRef]->width = $width;\n\n";
		
		print outputFile "/* Special sound file to play when Monkey lands on platform, if any */\n";
		print outputFile "currentWorld->platformRefs[nextPlatformRef]->snd = NULL;\n\n";
		
		print outputFile "/* Platform frequency */\n";
		print outputFile "currentWorld->platformRefs[nextPlatformRef]->frequency = $frequency;\n";
		print outputFile "currentWorld->frequencyTotal += currentWorld->platformRefs[nextPlatformRef]->frequency;\n\n";
		
		print outputFile "/* Increase platformRef count */\n";
		print outputFile "nextPlatformRef++;\n\n";
        
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
        #print outputFile pack("Aii/A*iiiii/A*i", "P", $platformType, $bmpName, $numFrames, $framePeriod, $offset, $width, $soundName, $frequency);
        print " - Platform reference:\n" if ($DEBUG);
        print "     - Type $platformType\n" if ($DEBUG);
        print "     - bmp file '$bmpName' with $numFrames frame(s), period $framePeriod\n" if ($DEBUG);
        print "     - Surface offset of $offset, width of $width.\n" if ($DEBUG);
        print "     - Sound file '$soundName'\n" if ($DEBUG);
        print "     - Frequency $frequency in world.\n" if ($DEBUG);

        print outputFile "/* PLATFORM TYPE $platformType: */\n\n";
        print outputFile "/* Create new */\n";
        print outputFile "currentWorld->platformRefs[nextPlatformRef] = (worldPlatformRef*) calloc(1,sizeof(worldPlatformRef));\n\n";
		print outputFile "/* Platform type */\n";
		print outputFile "currentWorld->platformRefs[nextPlatformRef]->typeCode = (platformType) $platformType;\n\n";
		print outputFile "/* Load animation */\n";
		print outputFile "loadAnimation(\"$bmpName\", $numFrames, &currentWorld->platformRefs[nextPlatformRef]->anim);\n\n";
		print outputFile "/* Frame period */\n";
		print outputFile "currentWorld->platformRefs[nextPlatformRef]->framePeriod = $framePeriod;\n\n";
		print outputFile "/* y offset of physical surface */\n";
		print outputFile "currentWorld->platformRefs[nextPlatformRef]->yOffset = $offset;\n\n";
		print outputFile "/* collision width of platform */\n";
		print outputFile "currentWorld->platformRefs[nextPlatformRef]->width = $width;\n\n";
		
		print outputFile "/* Special sound file to play when Monkey lands on platform, if any */\n";
		print outputFile "currentWorld->platformRefs[nextPlatformRef]->snd = loadSound(\"$soundName\");\n\n";
		
		print outputFile "/* Platform frequency */\n";
		print outputFile "currentWorld->platformRefs[nextPlatformRef]->frequency = $frequency;\n";
		print outputFile "currentWorld->frequencyTotal += currentWorld->platformRefs[nextPlatformRef]->frequency;\n\n";
		
		print outputFile "/* Increase platformRef count */\n";
		print outputFile "nextPlatformRef++;\n\n";


        next;
    }
    
    # Powerup frequencies
    if ( /^Powerup\stype\s(\d+)\sfrequency\s?=\s?((\d+)?\.?(\d+)?)(\s+)?$/ )
    {
        my $type = $1;
        my $frequency = $2;
        
        # Process into output file
        #print outputFile pack("Aif", "U", $type, $frequency);
        print " - Powerup type $type has frequency $frequency\n" if ($DEBUG);
        
		print outputFile "/* POWERUP */\n";
		print outputFile "/* Type */\n";
		print outputFile "currentWorld->powerups[nextPowerup] = (powerupType) $type;\n\n";

		print outputFile "/* Frequency */\n";
		print outputFile "currentWorld->powerupFrequencies[nextPowerup] = $frequency;\n\n";

		print outputFile "nextPowerup++;\n\n";
        
        next;
    }
        
    # Score to complete and unlock next level
    if ( /^Score\sto\scomplete\s?=\s?(\d+)(\s+)?$/ )
    {
        my $scoreToComplete=$1;
        
        # Process into output file
        #print outputFile pack("i", $scoreToComplete);
        print " - Score required to complete level: $scoreToComplete\n" if ($DEBUG);
        
		print outputFile "/* Score required to complete level */\n";
		print outputFile "currentWorld->completeScore = $scoreToComplete;\n\n";
        
        next;
    }
    
    
    # Scores required to get 1-5 stars
    if ( /^Star\sDistances\s?=\s?(\d+),\s?(\d+),\s?(\d+),\s?(\d+),\s?(\d+)(\s+)?$/ )
    {
        #print outputFile pack("iiiii", $1, $2, $3, $4, $5);
        print " - Scores required to get 1-5 stars: $1, $2, $3, $4, $5\n" if ($DEBUG);
        
		print outputFile "/* Scores required to get 1-5 stars */\n";
		print outputFile "currentWorld->starScore[0] = $1;\n";
		print outputFile "currentWorld->starScore[1] = $2;\n";
		print outputFile "currentWorld->starScore[2] = $3;\n";
		print outputFile "currentWorld->starScore[3] = $4;\n";
		print outputFile "currentWorld->starScore[4] = $5;\n\n";
        
        next;
    }
        
    
    
    # Start platform type
    if ( /^Start\splatform\stype\s?=\s?(\d+)(\s+)?$/ )
    {
        
        my $startPlatformType;
        
        $startPlatformType=$1;
        
        # Process into output file
        #print outputFile pack("i", $startPlatformType);
        print " - Start platform is of type $startPlatformType\n" if ($DEBUG);
        
		print outputFile "/* Type code of start platform */\n";
		print outputFile "currentWorld->startPlatform = (platformType) $startPlatformType;\n\n";
		
        next;
    }

    # Scrolling speed
    if ( /^Scrolling\sspeed\s?=\s?(\d+)(\s+)?$/ )
    {
        my $scrollingSpeed=$1;
        
        # Process into output file
        #print outputFile pack("i", $scrollingSpeed);
        print " - Scrolling speed is $scrollingSpeed\n" if ($DEBUG);
        
		print outputFile "/* Scrolling speed */\n";
		print outputFile "currentWorld->scrollSpeed = $scrollingSpeed;\n\n";
        
        next;
    }
    
    # Gravity
    if ( /^Gravity\s?=\s?((\d+)?\.?(\d+)?)(\s+)?$/ )
    {
        my $gravity=$1;
        
        # Process into output file
        #print outputFile pack("f", $gravity);
        print " - Gravity is $gravity\n" if ($DEBUG);
        
		print outputFile "/* Gravity */\n";
		print outputFile "currentWorld->gravity = $gravity;\n\n";
		
		print outputFile "/* Store this newly created world in the worlds array */\n";
		print outputFile "worlds[numWorlds] = currentWorld;\n\n";

		print outputFile "/* Increase variable holding running total number of worlds in game */\n";
		print outputFile "numWorlds++;\n";
		
        
        next;
    }
    
    warn "*** Could not parse line: $_\n" if ( !/^(\s+)?$/ );
        
}

print outputFile "}\n\n";

print "Finished processing.\n";