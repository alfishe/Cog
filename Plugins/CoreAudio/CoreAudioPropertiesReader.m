//
//  MADPropertiesReader.m
//  MAD
//
//  Created by Vincent Spader on 2/24/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import "CoreAudioPropertiesReader.h"
#import "CoreAudioDecoder.h"

@implementation CoreAudioPropertiesReader

- (NSDictionary *)propertiesForURL:(NSURL *)url
{
	NSDictionary *properties;
	CoreAudioDecoder *decoder;
	
	decoder = [[CoreAudioDecoder alloc] init];
	if (![decoder open:url])
	{
		return nil;
	}
	
	properties = [decoder properties];
	
	[decoder close];
	
	return properties;
}


+ (NSArray *)fileTypes
{
	return [CoreAudioDecoder fileTypes];
}

@end
