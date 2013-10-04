//
//  GameFile.h
//  Cog
//
//  Created by Vincent Spader on 5/29/06.
//  Copyright 2006 Vincent Spader. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import <GME/gme.h>

#import "Plugin.h"

extern gme_err_t readCallback( void* data, void* out, long count );

@interface GameDecoder : NSObject <CogDecoder> {
	Music_Emu* emu;
	id<CogSource> source;
	long length;
}

- (void)setSource:(id<CogSource>)s;
- (id<CogSource>)source;

@end
