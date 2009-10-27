#import "MainViewController.h"
#import <Foundation/NSTask.h>
#import <Foundation/NSPathUtilities.h>

@implementation MainViewController
@synthesize _logField, _fabrics, _runningButton, _stoppedButton, _runItem, _stopItem, _statusLabel;
@synthesize _addFabricButton;
@synthesize _app;

- (id) initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
	if(self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil]) {
		_outLock = [[NSLock alloc] init];
	}
	return self;
}

- (void)errorOutputAvailable: (NSNotification *)aNotification {
	[_outLock lock];
	NSData *taskData;
	NSString *newOutput;
	
	taskData = [[aNotification userInfo] objectForKey:@"NSFileHandleNotificationDataItem"];
	if([taskData length]) {
		newOutput = [[NSString alloc] initWithData:taskData encoding:NSMacOSRomanStringEncoding];
		
		NSDictionary* options = [NSDictionary dictionaryWithObjectsAndKeys:[NSFont fontWithName:@"Lucida Console" size:8.0], NSFontAttributeName, [NSColor redColor], NSForegroundColorAttributeName, nil];
		NSAttributedString* as = [[NSAttributedString alloc] initWithString:newOutput attributes:options];
		[_logField.textStorage insertAttributedString:as atIndex:[_logField.textStorage length]];
		[_logField scrollToEndOfDocument:nil];
		[as release];
		[newOutput release];
		[[[_app.task standardOutput] fileHandleForReading] readInBackgroundAndNotify];
	}
	[_outLock unlock];
}

- (void)outputAvailable: (NSNotification *)aNotification {
	[_outLock lock];
	NSData *taskData;
	NSString *newOutput;
	
	taskData = [[aNotification userInfo] objectForKey:@"NSFileHandleNotificationDataItem"];
	if([taskData length]) {
		newOutput = [[NSString alloc] initWithData:taskData encoding:NSMacOSRomanStringEncoding];
		
		NSDictionary* options = [NSDictionary dictionaryWithObjectsAndKeys:[NSFont fontWithName:@"Lucida Console" size:8.0], NSFontAttributeName, nil];
		NSAttributedString* as = [[NSAttributedString alloc] initWithString:newOutput attributes:options];
		[_logField.textStorage insertAttributedString:as atIndex:[_logField.textStorage length]];
		[_logField scrollToEndOfDocument:nil];
		[as release];
		[newOutput release];
		[[[_app.task standardOutput] fileHandleForReading] readInBackgroundAndNotify];
	}
	[_outLock unlock];
}

- (BOOL) validateToolbarItem:(NSToolbarItem *)theItem {
	if(theItem.tag==1) {
		return _app.task==nil;
	}
	else if(theItem.tag==2) {
		return _app.task!=nil;
	}
	return YES;
}

- (void) updateView {
	if(_app.task && [_app.task isRunning]) {
		[_statusLabel setTitleWithMnemonic:@"Server is running"];
		[_stoppedButton setHidden:YES];
		[_runningButton setHidden:NO];
		//[_addFabricButton setEnabled:FALSE];
	}
	else {
		[_statusLabel setTitleWithMnemonic:@""];
		[_stoppedButton setHidden:NO];
		[_runningButton setHidden:YES];
		//[_addFabricButton setEnabled:TRUE];
	}
};

- (void) processEnded: (NSNotification*)nt {
	[self updateView];
	_app.task = nil;
}

- (IBAction) clearLog: (id)sender {
	[_logField.textStorage replaceCharactersInRange:NSMakeRange(0, [_logField.textStorage length]) withString:@""];
}

- (IBAction) chooseFabricFile: (id)sender {
	if(_app.task==nil) {
		NSOpenPanel* openDlg = [NSOpenPanel openPanel];
		[openDlg setCanChooseFiles:YES];
		[openDlg setAllowsMultipleSelection:NO];
		
		if ([openDlg runModal] == NSOKButton) {
			NSMutableDictionary* obj = [[NSMutableDictionary alloc] init];
			NSString* fn = [openDlg filename];
			[obj setObject:fn forKey:@"Path"];
			[obj setObject:[fn lastPathComponent] forKey:@"Name"];
			[_fabrics addObject:obj];
			[obj release];
		}
	}
}

- (IBAction) startServer: (id)sender {
	if(!_app.task) {
		NSMutableArray* args = [[NSMutableArray alloc] init];
		//[args addObject:_fabricFileField.stringValue];
		for (NSMutableDictionary* obj in [_fabrics arrangedObjects]) {
			[args addObject:[obj valueForKey:@"Path"]];
		}
		_app.task = [[NSTask alloc] init];
		
		// Get path to the TJFabric executable
		NSBundle* mb = [NSBundle mainBundle];
		NSString* fabricBinaryPath = [mb pathForAuxiliaryExecutable:@"TJFabric"];
		NSString* fabricCD = [fabricBinaryPath stringByDeletingLastPathComponent];
		[_app.task setLaunchPath:fabricBinaryPath];
		[_app.task setArguments:args];
		[_app.task setCurrentDirectoryPath:fabricCD];
		
		NSPipe* outPipe = [[NSPipe alloc] init];
		NSPipe* errorPipe = [[NSPipe alloc] init];
		NSFileHandle* readOutPipe = [outPipe fileHandleForReading];
		NSFileHandle* readErrorPipe = [errorPipe fileHandleForReading];
		[_app.task setStandardOutput:outPipe];
		[_app.task setStandardError:errorPipe];
		
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(outputAvailable:) name:NSFileHandleReadCompletionNotification object:readOutPipe];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(errorOutputAvailable:) name:NSFileHandleReadCompletionNotification object:readErrorPipe];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(processEnded:) name:NSTaskDidTerminateNotification object:_app.task];
		
		[readOutPipe readInBackgroundAndNotify];
		[readErrorPipe readInBackgroundAndNotify];
		
		[_app.task launch];
		[args release];
		[outPipe release];
		[errorPipe release];
	}
	[self updateView];
}

- (IBAction) stopServer: (id)sender {
	if(_app!=nil) {
		[_app.task interrupt];
		[_app.task waitUntilExit];
		_app.task = nil;
	}
	
	[self updateView];
}

- (void) dealloc {
	[self stopServer:nil];
	[_statusLabel release];
	[_addFabricButton release];
	[_runItem release];
	[_stopItem release];
	[_runningButton release];
	[_stoppedButton release];
	[_logField release];
	[_fabrics release];
	[_outLock release];
	[super dealloc];
}

@end
