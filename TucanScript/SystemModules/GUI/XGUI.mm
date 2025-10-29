#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <objc/runtime.h>

#include "../../VirtualMachine.h"

#define REF_BUTTON_STATE "RefButtonState"

using namespace TucanScript;

@interface XGUIButtonFlag : NSObject
@property (nonatomic, assign) SInt32* m_pFlag;
@end

@implementation XGUIButtonFlag
@end

@interface XGUIApplicationDelegate : NSObject <NSApplicationDelegate>
@property (nonatomic, strong) NSWindow* m_hWindow;
@property BOOL m_WindowClosed;
@end

@implementation XGUIApplicationDelegate
@synthesize m_hWindow;
@synthesize m_WindowClosed;

- (instancetype)init {
    self = [super init];
    if (self) {
        m_WindowClosed = NO;
    }
    return self;
}

- (void) applicationDidFinishLaunching: (NSNotification *)aNotification {}
- (void) WindowWillClose: (NSNotification*) notification {
    self.m_WindowClosed = YES;
}
- (void) OnButtonClicked: (NSButton*) sender {
    XGUIButtonFlag* flag = objc_getAssociatedObject(sender, REF_BUTTON_STATE);
    if (flag) *(flag.m_pFlag) = 1;
}
@end

static XGUIApplicationDelegate* gs_Delegate = nil;

extern "C" {
TucanAPI Undef Frame(ExC_Args) {
    Dec32 fWidth = ExC_FloatArg(0);
    Dec32 fHeight = ExC_FloatArg(1);

    @autoreleasepool {
        if (NSApp == nil) {
            [NSApplication sharedApplication];
            [NSApp setActivationPolicy: NSApplicationActivationPolicyRegular];
        }

        if (!gs_Delegate) {
            gs_Delegate = [[XGUIApplicationDelegate alloc] init];
            [NSApp setDelegate: gs_Delegate];
        }

        NSRect screenRect = [[NSScreen mainScreen] frame];
        NSRect rect = NSMakeRect((screenRect.size.width - fWidth) * 0.5f,
                                 (screenRect.size.height - fHeight) * 0.5f,
                                 fWidth, fHeight);

        gs_Delegate.m_hWindow = [[NSWindow alloc]
                initWithContentRect:rect
                          styleMask: (NSWindowStyleMaskTitled |
                                      NSWindowStyleMaskClosable |
                                      NSWindowStyleMaskResizable)
                            backing:  NSBackingStoreBuffered
                              defer:  NO];

        NSString* pTitle = [NSString stringWithUTF8String: ExC_StringArg(2)];

        [gs_Delegate.m_hWindow setTitle: pTitle];
        [gs_Delegate.m_hWindow makeKeyAndOrderFront: nil];
        [NSApp activateIgnoringOtherApps: YES];

        [[NSNotificationCenter defaultCenter] addObserver: gs_Delegate
                                                 selector: @selector(WindowWillClose:)
                                                     name: NSWindowWillCloseNotification
                                                   object: gs_Delegate.m_hWindow];
    }
}

TucanAPI Undef IsAppRunning(ExC_Args) {
    stack->Push (gs_Delegate && !gs_Delegate.m_WindowClosed);
}

TucanAPI Undef Flush(ExC_Args) {
    @autoreleasepool {
        if (!NSApp) return;

        NSEvent *pEvent;
        while ((pEvent = [NSApp nextEventMatchingMask: NSEventMaskAny
                                            untilDate: [NSDate dateWithTimeIntervalSinceNow: Zero]
                                               inMode: NSDefaultRunLoopMode
                                              dequeue: YES])) {
            [NSApp sendEvent: pEvent];
        }
        [NSApp updateWindows];
    }
}

TucanAPI Undef TextField(ExC_Args) {
    @autoreleasepool {
        Dec32 fX         = ExC_FloatArg(0);
        Dec32 fY         = ExC_FloatArg(1);
        Dec32 fWidth     = ExC_FloatArg(2);
        Dec32 fHeight    = ExC_FloatArg(3);

        if (!gs_Delegate || !gs_Delegate.m_hWindow) {
            stack->Push((SInt32) _Fail);
            return;
        }

        __block NSTextField* hCreatedLabel = nil;

        void (^CreateLabelBlock)(void) = ^{
            NSTextField* hLabel = [[NSTextField alloc] initWithFrame: NSMakeRect(fX, fY, fWidth, fHeight)];
            TucanScript::DWord uLastIndex = 4;

            if (args->m_Size > uLastIndex) {
                if (args->m_Memory[uLastIndex].m_Type == TucanScript::VM::MANAGED_T) {
                    [hLabel setStringValue: [NSString stringWithUTF8String: ExC_StringArg (uLastIndex)]];
                    ++uLastIndex;
                }

                if (args->m_Size > uLastIndex) {
                    [hLabel setBezeled: (BOOL) ExC_Int32Arg (uLastIndex)];
                    ++uLastIndex;
                }

                if (args->m_Size > uLastIndex) {
                    [hLabel setDrawsBackground: (BOOL) ExC_Int32Arg (uLastIndex)];
                    ++uLastIndex;
                }

                if (args->m_Size > uLastIndex) {
                    [hLabel setEditable: (BOOL) ExC_Int32Arg (uLastIndex)];
                    ++uLastIndex;
                }

                if (args->m_Size > uLastIndex) {
                    [hLabel setSelectable: (BOOL) ExC_Int32Arg (uLastIndex)];
                    ++uLastIndex;
                }
            }

            NSView* hContentView = [gs_Delegate.m_hWindow contentView];
            if (hContentView) {
                [hContentView addSubview: hLabel];
                hCreatedLabel = hLabel;
            }
        };

        if ([NSThread isMainThread]) {
            CreateLabelBlock();
        } else {
            dispatch_sync(dispatch_get_main_queue(), CreateLabelBlock);
        }

        stack->Push<Undef*, VM::NATIVEPTR_T> ((Undef*) hCreatedLabel, &VM::Word::m_NativePtr);
    }
}

TucanAPI Undef TextField_SetText(ExC_Args) {
    @autoreleasepool {
        auto* hLabel = (NSTextField*) ExC_NativePtrArg(0);
        const Sym* pCStr = ExC_StringArg(1);

        if (!hLabel || !pCStr) {
            stack->Push((SInt32)_Fail);
            return;
        }

        void (^UpdateBlock)(void) = ^{
            [hLabel setStringValue:[NSString stringWithUTF8String: pCStr]];
        };

        if ([NSThread isMainThread]) {
            UpdateBlock();
        } else {
            dispatch_sync(dispatch_get_main_queue(), UpdateBlock);
        }

        stack->Push((SInt32) _Success);
    }
}

TucanAPI Undef TextField_GetText(ExC_Args) {
    @autoreleasepool {
        auto* field = (NSTextField*) ExC_NativePtrArg(0);

        if (!field) {
            stack->Push((SInt32)_Fail);
            return;
        }

        __block NSString* sResult = nil;

        void (^ReadBlock)(void) = ^{
            sResult = [field stringValue];
        };

        if ([NSThread isMainThread]) {
            ReadBlock();
        } else {
            dispatch_sync(dispatch_get_main_queue(), ReadBlock);
        }

        const Sym* pCStr = strdup([sResult UTF8String]);
        stack->Push<VM::Managed*, VM::MANAGED_T> (vm->GetAllocator()->Alloc ((Undef*) pCStr, [sResult length]), &VM::Word::m_ManagedPtr);
    }
}

TucanAPI Undef Button(ExC_Args) {
    @autoreleasepool {
        Dec32 fX = ExC_FloatArg(0);
        Dec32 fY = ExC_FloatArg(1);
        Dec32 fWidth = ExC_FloatArg(2);
        Dec32 fHeight = ExC_FloatArg(3);
        const char* pLabelContent = ExC_StringArg(4);

        auto* pPressedFlag = (SInt32*) ExC_NativePtrArg(5);

        __block NSButton* hButton = nil;

        void (^CreateButton)(void) = ^{
            hButton = [[NSButton alloc] initWithFrame: NSMakeRect(fX,fY,fWidth,fHeight)];
            [hButton setTitle: [NSString stringWithUTF8String: pLabelContent]];
            [hButton setBezelStyle: NSBezelStyleRounded];
            [hButton setButtonType: NSButtonTypeMomentaryPushIn];
            [hButton setTarget: gs_Delegate];
            [hButton setAction: @selector(OnButtonClicked:)];

            XGUIButtonFlag* flag = [XGUIButtonFlag new];
            flag.m_pFlag = pPressedFlag;
            objc_setAssociatedObject(hButton, REF_BUTTON_STATE, flag, OBJC_ASSOCIATION_RETAIN_NONATOMIC);

            [[gs_Delegate.m_hWindow contentView] addSubview: hButton];

            stack->Push<Undef*, VM::NATIVEPTR_T>((Undef*) hButton, &VM::Word::m_NativePtr);
        };

        if ([NSThread isMainThread]) {
            CreateButton();
        } else {
            dispatch_sync(dispatch_get_main_queue(), CreateButton);
        }
    }
}
}