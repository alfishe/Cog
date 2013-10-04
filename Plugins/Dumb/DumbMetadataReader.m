//
//  DumbMetadataReader.m
//  Dumb
//
//  Created by Vincent Spader on 10/12/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import "DumbMetadataReader.h"
#import "DumbDecoder.h"

#import <Dumb/dumb.h>

@implementation DumbMetadataReader

+ (NSArray *)fileTypes
{
	return [DumbDecoder fileTypes];
}

+ (NSArray *)mimeTypes
{
	return [DumbDecoder mimeTypes];
}

+ (NSDictionary *)metadataForURL:(NSURL *)url
{
    id audioSourceClass = NSClassFromString(@"AudioSource");
    id<CogSource> source = [audioSourceClass audioSourceForURL:url];
    
    if (![source open:url])
        return 0;
    
    if (![source seekable])
        return 0;

	DUMBFILE * df = dumbfile_open_ex(source, &dfs);
	if (!df)
	{
		NSLog(@"EX Failed");
		return NO;
	}
    
	DUH *duh;
	NSString *ext = [[[url path] pathExtension] lowercaseString];
    duh = dumb_read_any_quick(df, [ext isEqualToString:@"mod"] ? 0 : 1, 0);
    
    dumbfile_close(df);
    
	if (!duh)
	{
		NSLog(@"Failed to create duh");
		return nil;
	}

	//Some titles are all spaces?!
	NSString *title = [[NSString stringWithUTF8String: duh_get_tag(duh, "TITLE")] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
	unload_duh(duh);

	if (title == nil) {
		title = @"";
	}
	
	return [NSDictionary dictionaryWithObject:title forKey:@"title"];
}

@end
