//
//  main.cpp
//  SourceStatistic
//
//  Created by L&L on 2017/8/13.
//  Copyright © 2017年 L&L. All rights reserved.
//

#include <iostream>
#include "fileManager.hpp"

int main(int argc, const char * argv[]) {
    // insert code here...
    if (argc < 2)
    {
        printf("Not enough parameter!\n");
        return 0;
    }
    
    FileManager fm;
    fm.Run(argv[1]);
    
    return 0;
}
