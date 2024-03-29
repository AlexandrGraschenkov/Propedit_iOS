//
//  ViewController.m
//  TestProperEdit
//
//  Created by Alexander Graschenkov on 11.04.2020.
//  Copyright © 2020 Alex the Best. All rights reserved.
//

#import "ViewController.h"
#include <MKVProperEdit/propedit.h>
#include <string>

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    NSString *docPath = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES)[0];
    NSString *vidPath = [docPath stringByAppendingPathComponent:@"friends.mp4"];
    BOOL exist = [[NSFileManager defaultManager] fileExistsAtPath:vidPath];
    NSLog(@"doc %@ %@", vidPath, exist ? @"YES" : @"NO");
    
    NSMutableArray<NSString *> *command = @[
        @"mkvpropedit",
        vidPath,
        @"--edit",
        @"track:a1",
        @"--set",
        @"flag-default=0",
        @"--edit",
        @"track:a2",
        @"--set",
        @"flag-default=1",
        @"-v"];
    
    char **data = new char*[command.count];// (char**) malloc(sizeof(char*) * command.count);
    for (int i = 0; i < command.count; i++) {
        const char *val = [command[i] UTF8String];
        int count = command[i].length + 1;
        data[i] = new char[count];
        std::strncpy(data[i], val, count);
    }
    double d1 = CACurrentMediaTime();
    run_edit(command.count, data);
    double d2 = CACurrentMediaTime();
    
    NSLog(@"Test %lf", d2-d1);
}


@end
