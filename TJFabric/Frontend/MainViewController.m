#import "MainViewController.h"
#import <Foundation/NSTask.h>
#import <Foundation/NSPathUtilities.h>

@implementation MainViewController
@synthesize _logField, _fabricFileField, _runningButton, _stoppedButton, _runItem, _stopItem;
@synthesize task = _task;

- (void)errorOutputAvailable: (NSNotification *)aNotification {
	NSData *taskData;
	NSString *newOutput;
	
	taskData = [[aNotification userInfo] objectForKey:@"NSFileHandleNotificationDataItem"];
	if([taskData length]) {
		newOutput = [[NSString alloc] initWithData:taskData encoding:NSMacOSRomanStringEncoding];
		
		NSDictionary* options = [NSDictionary dictionaryWithObjectsAndKeys:[NSFont fontWithName:@"Lucida Console" size:10.0], NSFontAttributeName, [NSColor redColor], NSForegroundColorAttributeName, nil];
		NSAttributedString* as = [[NSAttributedString alloc] initWithString:newOutput attributes:options];
		[_logField.textStorage insertAttributedString:as atIndex:[_logField.textStorage length]];
		[_logField scrollToEndOfDocument:nil];
		[as release];
		[newOutput release];
		[[[self.task standardOutput] fileHandleForReading] readInBackgroundAndNotify];
	}
}

- (void)outputAvailable: (NSNotification *)aNotification {
	NSData *taskData;
	NSString *newOutput;
	
	taskData = [[aNotification userInfo] objectForKey:@"NSFileHandleNotificationDataItem"];
	if([taskData length]) {
		newOutput = [[NSString alloc] initWithData:taskData encoding:NSMacOSRomanStringEncoding];
		
		NSDictionary* options = [NSDictionary dictionaryWithObjectsAndKeys:[NSFont fontWithName:@"Lucida Console" size:10.0], NSFontAttributeName, nil];
		NSAttributedString* as = [[NSAttributedString alloc] initWithString:newOutput attributes:options];
		[_logField.textStorage insertAttributedString:as atIndex:[_logField.textStorage length]];
		[_logField scrollToEndOfDocument:nil];
		[as release];
		[newOutput release];
		[[[self.task standardOutput] fileHandleForReading] readInBackgroundAndNotify];
	}
}

- (BOOL) validateToolbarItem:(NSToolbarItem *)theItem {
	if(theItem.tag==1) {
		return _task==nil;
	}
	else if(theItem.tag==2) {
		return _task!=nil;
	}
	return FALSE;
}

- (void) updateView {
	if(self.task && [_task isRunning]) {
		[_stoppedButton setHidden:YES];
		[_runningButton setHidden:NO];
	}
	else {
		[_stoppedButton setHidden:NO];
		[_runningButton setHidden:YES];
	}
};

- (void) processEnded: (NSNotification*)nt {
	[self updateView];
	self.task = nil;
}

- (IBAction) chooseFabricFile: (id)sender {
	if(self.task==nil) {
		NSOpenPanel* openDlg = [NSOpenPanel openPanel];
		[openDlg setCanChooseFiles:YES];
		[openDlg setAllowsMultipleSelection:NO];
		
		if ([openDlg runModal] == NSOKButton) {
			_fabricFileField.stringValue = [openDlg filename];
		}
	}
}

- (IBAction) startServer: (id)sender {
	if(!self.task) {
		NSMutableArray* args = [[NSMutableArray alloc] init];
		[args addObject:_fabricFileField.stringValue];
		self.task = [[NSTask alloc] init];
		
		// Get path to the TJFabric executable
		NSBundle* mb = [NSBundle mainBundle];
		NSString* fabricBinaryPath = [mb pathForAuxiliaryExecutable:@"TJFabric"];
		NSString* fabricCD = [fabricBinaryPath stringByDeletingLastPathComponent];
		[self.task setLaunchPath:fabricBinaryPath];
		[self.task setArguments:args];
		[self.task setCurrentDirectoryPath:fabricCD];
		
		NSPipe* outPipe = [[NSPipe alloc] init];
		NSPipe* errorPipe = [[NSPipe alloc] init];
		NSFileHandle* readOutPipe = [outPipe fileHandleForReading];
		NSFileHandle* readErrorPipe = [errorPipe fileHandleForReading];
		[self.task setStandardOutput:outPipe];
		[self.task setStandardError:errorPipe];
		
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(outputAvailable:) name:NSFileHandleReadCompletionNotification object:readOutPipe];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(errorOutputAvailable:) name:NSFileHandleReadCompletionNotification object:readErrorPipe];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(processEnded:) name:NSTaskDidTerminateNotification object:self.task];
		
		[readOutPipe readInBackgroundAndNotify];
		[readErrorPipe readInBackgroundAndNotify];
		
		[self.task launch];
		[args release];
		[outPipe release];
		[errorPipe release];
	}
	[self updateView];
}

- (IBAction) stopServer: (id)sender {
	if(_task!=nil) {
		[_task interrupt];
		[_task waitUntilExit];
		self.task = nil;
	}
	
	[self updateView];
}

- (void) dealloc {
	[self stopServer:nil];
	[_runItem release];
	[_stopItem release];
	[_runningButton release];
	[_stoppedButton release];
	[_logField release];
	[_fabricFileField release];
	[super dealloc];
}

@end
