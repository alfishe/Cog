//
//  PreferencesController.m
//  Preferences
//
//  Created by Vincent Spader on 9/4/06.
//  Copyright 2006 Vincent Spader. All rights reserved.
//

#import "GeneralPreferencesPlugin.h"
#import "PathToFileTransformer.h"

@implementation GeneralPreferencesPlugin

+ (void)initialize
{
	NSValueTransformer *pathToFileTransformer = [[PathToFileTransformer alloc] init];
    [NSValueTransformer setValueTransformer:pathToFileTransformer
                                    forName:@"PathToFileTransformer"];
}

+ (NSArray *)preferencePanes
{
	GeneralPreferencesPlugin *plugin = [[GeneralPreferencesPlugin alloc] init];
	[NSBundle loadNibNamed:@"Preferences" owner:plugin];
	
	return [NSArray arrayWithObjects:
			[plugin playlistPane],
			[plugin hotKeyPane],
			[plugin updatesPane],
			[plugin outputPane],
			[plugin scrobblerPane],
            [plugin growlPane],
            [plugin appearancePane],
            [plugin midiPane],
			nil];
}	

- (HotKeyPane *)hotKeyPane
{
	return hotKeyPane;
}

- (OutputPane *)outputPane
{
	return outputPane;
}

- (MIDIPane *)midiPane
{
    return midiPane;
}

- (GeneralPreferencePane *)updatesPane
{
	return [GeneralPreferencePane preferencePaneWithView:updatesView title:NSLocalizedStringFromTableInBundle(@"Updates", nil, [NSBundle bundleForClass:[self class]], @"")  iconNamed:@"updates"];
}

- (GeneralPreferencePane *)scrobblerPane
{
	return [GeneralPreferencePane preferencePaneWithView:scrobblerView title:NSLocalizedStringFromTableInBundle(@"Last.fm", nil, [NSBundle bundleForClass:[self class]], @"")  iconNamed:@"lastfm"];
}

- (GeneralPreferencePane *)playlistPane
{
	return [GeneralPreferencePane preferencePaneWithView:playlistView title:NSLocalizedStringFromTableInBundle(@"Playlist", nil, [NSBundle bundleForClass:[self class]], @"")  iconNamed:@"playlist"];
}

- (GeneralPreferencePane *)growlPane
{
    return [GeneralPreferencePane preferencePaneWithView:growlView title:NSLocalizedStringFromTableInBundle(@"Growl", nil, [NSBundle bundleForClass:[self class]], @"")  iconNamed:@"growl"];
}

- (GeneralPreferencePane *)appearancePane
{
    return [GeneralPreferencePane preferencePaneWithView:appearanceView title:NSLocalizedStringFromTableInBundle(@"Appearance", nil, [NSBundle bundleForClass:[self class]], @"")  iconNamed:@"appearance"];
}

@end
