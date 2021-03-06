//
//  MIDIPane.m
//  General
//
//  Created by Christopher Snowhill on 10/15/13.
//
//

#import "MIDIPane.h"

@implementation MIDIPane

- (void)awakeFromNib
{
    if ([[[NSUserDefaults standardUserDefaults] stringForKey:@"midi.plugin"] isEqualToString:@"Sc55rolD"])
        [midiFlavorControl setEnabled:YES];
}

- (NSString *)title
{
	return NSLocalizedStringFromTableInBundle(@"Synthesis", nil, [NSBundle bundleForClass:[self class]], @"");
}

- (NSImage *)icon
{
	return [[NSImage alloc] initWithContentsOfFile:[[NSBundle bundleForClass:[self class]] pathForImageResource:@"midi"]];
}

- (IBAction)setSoundFont:(id)sender
{
    NSArray *fileTypes = [NSArray arrayWithObjects:@"sf2",@"sf2pack",@"sflist",@"json",nil];
    NSOpenPanel * panel = [NSOpenPanel openPanel];
    [panel setAllowsMultipleSelection:NO];
    [panel setCanChooseDirectories:NO];
    [panel setCanChooseFiles:YES];
    [panel setFloatingPanel:YES];
    [panel setAllowedFileTypes:fileTypes];
    NSString * oldPath = [[NSUserDefaults standardUserDefaults] stringForKey:@"soundFontPath"];
    if ( oldPath != nil )
        [panel setDirectoryURL:[NSURL fileURLWithPath:oldPath]];
    NSInteger result = [panel runModal];
    if(result == NSOKButton)
    {
        [[NSUserDefaults standardUserDefaults] setValue:[[panel URL] path] forKey:@"soundFontPath"];
    }
}

- (IBAction)setMidiPlugin:(id)sender
{
    if ([[[NSUserDefaults standardUserDefaults] stringForKey:@"midi.plugin"] isEqualToString:@"Sc55rolD"])
        [midiFlavorControl setEnabled:YES];
    else
        [midiFlavorControl setEnabled:NO];
}

@end
