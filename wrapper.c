//
// Created by tntrol on 10.08.2021.
//

#include "wrapper.h"
#include "scanner.h"

void scanning_server_wrapper(char *url, void(*callback) (Report *report))
{
    Report *report = scan_server(url);
    if(report)
    {
        callback(report);
        free(report);
    }
}