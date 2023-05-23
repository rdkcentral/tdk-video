/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2023 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <linux/unistd.h>

#define LOOP_ITERATION        100000
int main(int argc, char *argv[])
{
        int file_access, i, success=0, fail=0;
        char *compiled_file = "/bin/ls";
        char *permission = "r";
        char *read_only = "r";
        char *write_only = "w";
        char *read_write = "rw";
        char *read_write_inquire = "rix";

        if (argc > 1)
                compiled_file = argv[1];
        if (argc > 2)
                permission = argv[2];
        
	for (i=0; i<LOOP_ITERATION; i++)
        {
                if (!strcmp(permission,read_only))
                {
                    file_access = open(compiled_file, O_RDONLY);
                }
                else if (!strcmp(permission, write_only))
                {
                    file_access = open(compiled_file, O_WRONLY);
                }
                else if (!strcmp(permission, read_write))
                {
                    file_access = open(compiled_file, O_RDWR);
                }
                else if (!strcmp(permission, read_write_inquire))
                {
                    file_access = open(compiled_file, O_RDONLY | O_CLOEXEC);
                }
                if (file_access != -1)
                {
                        success++;
                        close(file_access);
                }
                else
                {
                        printf("Failed sat iteration %d , with error : %s\n",i+1,strerror(errno));
			fail++;
                        break;
                }
        }
        printf("Iterations: %d\nSuccess: %d\nFail: %d\n",LOOP_ITERATION, success, fail);
        return 0;
}
