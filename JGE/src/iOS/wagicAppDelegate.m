#import "wagicAppDelegate.h"
#import "EAGLView.h"
#import "EAGLViewController.h"
#import "ASIHTTPRequest.h"
#import "ZipArchive.h"

@implementation wagicAppDelegate

@synthesize window;
@synthesize glViewController;
@synthesize wagicDownloadController;
@synthesize hostReach, wifiReach, internetReach;


- (void) downloadResources
{
    wagicDownloadController = [[WagicDownloadProgressViewController alloc] init];

    [self.window addSubview: wagicDownloadController.view];
    [self.window makeKeyWindow];

}

/**
 check for any zip files dropped into the documents directory.  If so move them into the "Res" directory.
 check for a "core" zip file in the Res directory. If it exists, then return YES. Otherwise, return NO.
 */
- (BOOL) hasResourceFiles
{
    NSFileManager *fileManager = [NSFileManager defaultManager];
    
    NSArray *paths = NSSearchPathForDirectoriesInDomains( NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *docsPath = [paths objectAtIndex: 0];
    NSArray *docsPathContents = [[NSFileManager defaultManager] contentsOfDirectoryAtPath: docsPath error:nil];
    NSArray *resourceZipFiles = [docsPathContents filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"self ENDSWITH '.zip'"]];
    NSString *resPath = [NSString stringWithFormat: @"%@/Res", docsPath];
    NSError *error = nil;
    for (NSString *zipFile in resourceZipFiles)
    {
        NSString *oldPath = [NSString stringWithFormat: @"%@/%@", docsPath, zipFile];
        NSString *newPath = [NSString stringWithFormat: @"%@/%@", resPath, zipFile];
        
        [fileManager moveItemAtPath: oldPath toPath:newPath error: &error];
        if ( error != nil )
        {
            NSLog(@"Error happened while trying to move %@ to %@: \n%@", oldPath, newPath, [error localizedDescription]);
        }
    }
    
    NSArray *resDirContents = [[NSFileManager defaultManager] contentsOfDirectoryAtPath: resPath error:nil];
    NSArray *coreFiles = [resDirContents filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"self BEGINSWITH 'core_'"]];
    if ([coreFiles count] > 0)
    {
        return YES;
    }
    
    return NO;
}


- (void) startGame
{
    if (glViewController != nil) 
        [glViewController release];
    glViewController = [[EAGLViewController alloc] init];
    
    if (wagicDownloadController != nil)
        [wagicDownloadController release];

    [self.window addSubview:self.glViewController.view];

    NSNotificationCenter *dnc = [NSNotificationCenter defaultCenter];
	[dnc removeObserver: self name: @"readyToStartGame" object: nil];
}


- (void)dealloc
{
    [window release];
    [glViewController release];
    [hostReach release];
    [wifiReach release];
    [internetReach release];
    
    [super dealloc];
}


- (void) setupNetworkListeners
{
    hostReach = [[Reachability reachabilityForGoogleDNS] retain];
    internetReach = [[Reachability reachabilityForInternetConnection] retain];
    wifiReach = [[Reachability reachabilityForLocalWiFi] retain];
    
    [hostReach startNotifier];
    [internetReach startNotifier];
    [wifiReach startNotifier];
}


- (BOOL) application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions 
{
    self.glViewController = nil;
    
    [self setupNetworkListeners];

    NSNotificationCenter *dnc = [NSNotificationCenter defaultCenter];
	[dnc addObserver:self selector:@selector(startGame) name:@"readyToStartGame" object: nil];
    
    // check to see if the Res folder exists.  If it does continue
    // otherwise bring up the download dialog and download the core files
    // once downloaded, extract the files and kick off the game.
    BOOL requiresResourceUpdate = [self hasResourceFiles];
    if (!requiresResourceUpdate)
    {
        [self downloadResources];
    }
    else
    {
        [self startGame];
    }
    
    [self.window setBackgroundColor: [UIColor blackColor]];
    [self.window makeKeyAndVisible];

    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    if ( [self.glViewController.view respondsToSelector: @selector(stopAnimation)])
        [self.glViewController.view stopAnimation];
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    if ( [self.glViewController.view respondsToSelector: @selector(stopAnimation)])
        [self.glViewController.view startAnimation];
}


- (void)applicationWillEnterForeground:(UIApplication *)application
{
    if ( [self.glViewController.view respondsToSelector: @selector(stopAnimation)])
        [self.glViewController.view startAnimation];
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    if ( [self.glViewController.view respondsToSelector: @selector(stopAnimation)])
        [self.glViewController.view stopAnimation];
}


- (void)applicationWillTerminate:(UIApplication *)application
{
    if ( [self.glViewController.view respondsToSelector: @selector(stopAnimation)])
        [self.glViewController.view stopAnimation];
}

- (void)initializeKeyboard: (id) initialState
{
    [self.glViewController toggleKeyboardWithState: initialState];
}

- (void) handleWEngineCommand:(NSString *) command 
             withUIParameters: (CGFloat) x 
                  yCoordinate: (CGFloat) y 
                        width: (CGFloat) width 
                       height: (CGFloat) height
{
}

- (void)handleWEngineCommand:(NSString *) command withParameter: (NSString *) parameter
{
    BOOL isDevicePhone = (UI_USER_INTERFACE_IDIOM()) == UIUserInterfaceIdiomPhone;

    if ([command isEqualToString: @"entergamestate:menu"] )
        [glViewController.eaglView displayAds];
    
    else if ([command isEqualToString: @"enterduelphase:end"] && isDevicePhone)
        [glViewController.eaglView displayAds];
    
    else if ([command isEqualToString: @"leaveduelphase:end"] || 
                [command isEqualToString: @"leavegamestate:menu"])
    {
        if (isDevicePhone)
            [glViewController.eaglView removeAds];
    }
    else if ([command isEqualToString: @"displayKeyboard"])
    {
        [self initializeKeyboard: parameter];
    }
    else if ([command isEqualToString: @"combatGuiEndDamage"])
    {
        [glViewController removeOkButtonListener];
    }
}


- (void) rotateBackgroundImage:(UIInterfaceOrientation)fromInterfaceOrientation toInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
	bool isPhone = (UI_USER_INTERFACE_IDIOM()) == UIUserInterfaceIdiomPhone;
    
	if (isPhone) 
    {
        UIImage *bgImage = [UIImage imageNamed: @"Default-Portrait.png"];
        [[[self.window subviews] objectAtIndex: 0] setBackgroundColor: [UIColor colorWithPatternImage: bgImage]];
	}
	else 
    {
        [self.window setBackgroundColor: [UIColor clearColor]];
		if (UIInterfaceOrientationIsLandscape( toInterfaceOrientation)) {
            UIImage *bgImage = [UIImage imageNamed: @"Default-Landscape"];
            [[[self.window subviews] objectAtIndex: 0] setBackgroundColor: [UIColor colorWithPatternImage: bgImage]];
		}
		else {
            UIImage *bgImage = [UIImage imageNamed: @"Default-Portrait.png"];
            [[[self.window subviews] objectAtIndex: 0] setBackgroundColor: [UIColor colorWithPatternImage: bgImage]];
            [self.window setBackgroundColor: [UIColor colorWithPatternImage: bgImage]];
		}
	}
}


#pragma mark Application Contents

- (BOOL) isNetworkAvailable
{
	BOOL netAvailable = NO;
    //NSDate *startTime = [[[NSDate alloc ] init] autorelease];
	
    hostReach = [[Reachability reachabilityForGoogleDNS] retain];
    
    NetworkStatus netStatus = [hostReach currentReachabilityStatus];
    
	
	if (netStatus == ReachableViaWiFi || netStatus == ReachableViaWWAN)	{
		netAvailable = YES;
	}
    
    //[[QLog log] logOption: kLogOptionTiming withFormat:@"isNetworkAvailble? %5.2f", [startTime timeIntervalSinceNow]];
	
	[hostReach release];
	
	return netAvailable;
}


#pragma mark -

@end
