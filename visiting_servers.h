//
// Created by tntrol on 17.08.2021.
//

#ifndef SUMMERPRACTICE_VISITING_SERVERS_H
#define SUMMERPRACTICE_VISITING_SERVERS_H

#include "utils.h"

int threading_visit_with_thread_pool(int size, char **urls, Report **out_reports);
int threading_visit(int size, char **urls, Report **out_reports);
int serial_visit(int size, char **urls, Report **out_reports);

#endif //SUMMER_PRACTICE_VISITING_SERVERS_H
