//
//  PlsContainer.m
//  Pls
//
//  Created by Zaphod Beeblebrox on 10/8/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import "PlsContainer.h"


@implementation PlsContainer

+ (NSArray *)fileTypes
{
	return [NSArray arrayWithObject:@"pls"];
}

+ (NSArray *)mimeTypes
{
	return [NSArray arrayWithObjects:@"audio/x-scpls", @"application/pls", nil];
}

+ (NSURL *)urlForPath:(NSString *)path relativeTo:(NSString *)baseFilename
{
	if ([path hasPrefix:@"/"]) {
		return [NSURL fileURLWithPath:path];
	}
	
	NSRange foundRange = [path rangeOfString:@"://"];
	if (foundRange.location != NSNotFound) 
	{
		return [NSURL URLWithString:path];
	}
	
	NSString *basePath = [[[baseFilename stringByStandardizingPath] stringByDeletingLastPathComponent] stringByAppendingString:@"/"];
	NSMutableString *unixPath = [path mutableCopy];
	
	//Only relative paths would have windows backslashes.
	[unixPath replaceOccurrencesOfString:@"\\" withString:@"/" options:0 range:NSMakeRange(0, [unixPath length])];
	
	return [NSURL fileURLWithPath:[basePath stringByAppendingString:[unixPath autorelease]]];
}

+ (NSArray *)urlsForContainerURL:(NSURL *)url
{
	if (![url isFileURL]) 
		return [NSArray array];
	
	NSString *filename = [url path];
	
	NSError *error;
	NSString *contents = [NSString stringWithContentsOfFile:filename encoding:NSUTF8StringEncoding error:&error];
	if (error || !contents) {
		return nil;
	}
	
	NSString *entry;
	NSEnumerator *e = [[contents componentsSeparatedByString:@"\n"] objectEnumerator];
	NSMutableArray *entries = [NSMutableArray array];
	
    while (entry = [[e nextObject] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]])
	{
		NSScanner *scanner = [[NSScanner alloc] initWithString:entry];
		NSString *lhs = nil;
		NSString *rhs = nil;
		
		if (![scanner scanUpToString:@"=" intoString:&lhs]	|| // get LHS
			![scanner scanString:@"=" intoString:nil]		|| // skip the =
			![scanner scanUpToString:@"" intoString:&rhs]	|| // get RHS
			![lhs isEqualToString:@"File"]) // We only want file entries
		{
			[scanner release];
			continue;
		}
		
		//need to add basepath if its a file, and convert to URL
		[entries addObject:[self urlForPath:rhs relativeTo:filename]];
		
		[scanner release];
	}
	
	return entries;
}	

@end