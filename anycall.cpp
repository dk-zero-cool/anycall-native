/*
 * Copyright 2016 xiaokangwang
 * Copyright 2016-2017 Alex Zhang aka. ztc1997
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define LOG_TAG "anycall"

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <binder/IBinder.h>
#include <binder/BpBinder.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>
#include <utils/Errors.h>

#include "base64.h"
#include "anycall.h"

using namespace android;

/**
 * Make RPC calls to any Android system server from a shell, e.g. root
 *
 * __Single Mode__
 *      -> anycall ServerName TransactionCode EncodedParcel
 *
 * __Daemon Mode__
 *      -> anycall ServerName
 *      -> TransactionCode EncodedParcel
 *      -> exit
 *
 * The EncodedParcel must be a Base64 encoded String. Both modes will print
 * a reply Parcel, also as Base64 encoded String.
 *
 * This binary is build to work together with a shell process.
 * As such it rapports back on anything, both using a result code but
 * also by printing the name of the result code preprocessor constant.
 *
 * This makes it very easy to find suitable ways to monitor the state of a call.
 * It also has a ping request build into daemon mode so that a shell process
 * can always check to see if it's still alive.
 */
int main(int argc, char **argv) {
    /* We need at least a service name to start in daemon mode.
    If we wanted single mode, we also need a transaction code and an encoded parcel */
    if (argc < 2) {
        printf("ERROR_MISSING_PARAMETERS\n");
        return ERROR_MISSING_PARAMETERS;
    }

    /* We can't get a service without first connecting to the service manager */
    sp<IServiceManager> sm = defaultServiceManager();
    if (sm == 0) {
        printf("ERROR_SERVICE_MANAGER\n");
        return ERROR_SERVICE_MANAGER;
    }

    /* We have the service manager, let's request the service that we wish to connect to */
    String16 service = String16(argv[1]);
    sp<IBinder> binder = sm->getService(service);
    if (binder == 0) {
        printf("ERROR_SERVICE_BINDER\n");
        return ERROR_SERVICE_BINDER;
    }

    char* input;
    size_t capacity;
    bool daemon = argc == 2;

    /* If daemon mode is to be started, allocate a buffer for incomming messages */
    if (daemon) {
        printf("INFO_DAEMON_MODE\n");
        capacity = 256;
        input = new char[capacity];
    }

    /* Start loop. If single mode is used, it will break out after the RPC Call.
    If daemon mode is set to be started, this will continue */
    for (;;) {
        /* Transaction code */
        int code;

        /* Encoded Parcel */
        char* data;

        /* Enter daemon mode, start listening for encomming transactions */
        if (daemon) {
            fgets(input, capacity, stdin);

            /* We started above by reading into a static buffer.
            Check to see if this buffer needs to be expanded and then read the rest of the message */
            for (;;) {
                size_t len = strlen(input);

                if (len == capacity-1 && input[len-1] != '\n') {
                    size_t cap = (capacity*2)-1; // Extract one NULL character from then length, still only need one, not two
                    char* buffer = new char[cap];

                    strcpy(buffer, input);
                    fgets(input, capacity, stdin);
                    strcat(buffer, input);

                    input = buffer;
                    capacity = cap;

                } else {
                    break;
                }
            }

            /* Look for Ping or Exit requests */
            if (input[4] == '\n') {
                if (input[0] == 'e' && strcmp(strtok(input, "\n"), "exit") == 0) {
                    printf("INFO_DAEMON_STOPPED\n");
                    break;

                } else if (input[0] == 'p' && strcmp(strtok(input, "\n"), "ping") == 0) {
                    printf("INFO_DAEMON_RUNNING\n");
                    continue;
                }
            }

            /* Extract transaction code from message */
            char* codeStr = strtok(input, " ");
            if (codeStr == NULL) {
                printf("ERROR_INVALID_INPUT\n");
                continue;
            }

            /* Extract encoded Parcel code from message */
            code = atoi(codeStr);
            data = strtok(NULL, "\n");
            if (data == NULL) {
                printf("ERROR_INVALID_INPUT\n");
                continue;
            }

            /* End Daemon mode message setup */

        /* Enter single mode */
        } else {
            /* Make sure the argument length is valid. */
            if (argc != 4) {
                printf("ERROR_INVALID_INPUT\n");
                return ERROR_INVALID_INPUT;
            }

            /* Extract transaction code */
            code = atoi(argv[2]);

            /* Extract encoded Parcel */
            data = argv[3];

            /* End single mode message setup */
        }

        /* Decode the extracted Parcel */
        size_t size;
        unsigned char* result = base64_decode(reinterpret_cast<const unsigned char*>(data),
                                              strlen(data), &size);

        /* Make sure that the encoded Parcel was valid */
        if (result == NULL) {
            printf("ERROR_DECODE_PARCEL\n");

            if (daemon) {
                continue;

            } else {
                return ERROR_DECODE_PARCEL;
            }
        }

        Parcel request, reply;

        request.setDataSize(size);
        request.setDataPosition(0);

        /* Convert the decoded Parcel data into a real Parcel */
        void* raw = request.writeInplace(size);
        memmove(raw, reinterpret_cast<const void*>(result), size);

        /* Make the transaction, call the service */
        status_t st = binder->remoteBinder()->transact(code, request, &reply);

        /* Make sure that the transaction was handled successfully */
        if (st != NO_ERROR) {
            printf("ERROR_CALL_FAILED\n");

            if (daemon) {
                continue;

            } else {
                return ERROR_CALL_FAILED;
            }
        }

        /* Encode the reply Parcel so that we can return it to caller */
        unsigned char* output = base64_encode(reply.data(), reply.dataSize(), &size);

        /* Check that the reply parcel was encoded successfully */
        if (output == NULL) {
            printf("ERROR_ENCODE_PARCEL\n");

            if (daemon) {
                continue;

            } else {
                return ERROR_DECODE_PARCEL;
            }
        }

        /* Return the reply to caller */
        printf("parcel:%s\n", output);

        /* Quit if this is a single mode request */
        if (!daemon) {
            break;
        }
    }

    return 0;
}
