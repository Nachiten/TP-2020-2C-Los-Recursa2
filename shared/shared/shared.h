/*
 * shared.h
 *
 *  Created on: 29 ago. 2020
 *      Author: utnso
 */

#ifndef SHARED_SHARED_H_
#define SHARED_SHARED_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h> // Para tener el uint32_t
#include "commons/log.h"
#include "commons/string.h"
#include "commons/config.h"
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <readline/readline.h>
#include <commons/collections/list.h>

void probarLasShared();

#endif /* SHARED_SHARED_H_ */
