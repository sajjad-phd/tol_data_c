/*
    Minimal utility functions for DAQ HAT C examples
    This file contains only the functions needed for channel4_ringbuffer_logger.c
*/

#ifndef UTILITY_H_
#define UTILITY_H_

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <stdint.h>
#include <daqhats/daqhats.h>

// Channel definitions
#define CHAN0 0x01 << 0
#define CHAN1 0x01 << 1
#define CHAN2 0x01 << 2
#define CHAN3 0x01 << 3
#define CHAN4 0x01 << 4
#define CHAN5 0x01 << 5
#define CHAN6 0x01 << 6
#define CHAN7 0x01 << 7

// Read definitions
#define READ_ALL_AVAILABLE  -1

/****************************************************************************
 * Display functions
 ****************************************************************************/
/* This function takes a result code as the result parameter and if the
   result code is not RESULT_SUCCESS, the error message is sent to stderr. */
void print_error(int result)
{
    if (result != RESULT_SUCCESS)
    {
        fprintf(stderr, "\nError: %s\n", hat_error_message(result));
    }
}

/****************************************************************************
 * User input functions
 ****************************************************************************/
void flush_stdin(void)
{
    int c;

    do
    {
        c = getchar();
    } while (c != '\n' && c != EOF);
}

/* This function displays the available DAQ HAT devices and allows the user
   to select a device to use with the associated example.  The address
   parameter, which is passed by reference, is set to the selected address.
   The return value is 0 for success and -1 for error.*/
int select_hat_device(uint16_t hat_filter_id, uint8_t* address)
{
    struct HatInfo* hats = NULL;
    int hat_count = 0;
    int address_int = 0;
    int return_val = -1;
    int i;

    if (address == NULL)
        return -1;

    // Get the number of HAT devices that are connected that are of the
    // requested type.
    hat_count = hat_list(hat_filter_id , NULL);

    // Verify there are HAT devices connected that are of the requested type.
    if (hat_count > 0)
    {
        // Allocate memory for the list of HAT devices.
        hats = (struct HatInfo*)malloc(hat_count * sizeof(struct HatInfo));

        // Get the list of HAT devices.
        hat_list(hat_filter_id, hats);

        if (hat_count == 1)
        {
            // Get the address of the only HAT device.
            *address = hats[0].address;
            return_val = 0;
        }
        else
        {
            // There is more than 1 HAT device so display the
            // list of devices and let the user choose one.
            for (i = 0; i < hat_count; i++)
            {
                printf("Address %d: %s\n", hats[i].address,
                    hats[i].product_name);
            }

            printf("\nSelect the address of the HAT device to use: ");
            if (scanf("%d", &address_int) == 0)
            {
                fprintf(stderr, "Error: Invalid selection\n");
                free(hats);
                return -1;
            }

            *address = (uint8_t)address_int;

            // Find the HAT device with the specified address in the list.
            for (i = 0; i < hat_count; i++)
            {
                if (hats[i].address == *address)
                {
                    return_val = 0;
                    break;
                }
            }

            if (return_val != 0)
            {
                fprintf(stderr, "Error: Invalid HAT address\n");
            }
            flush_stdin();
        }

        // Release the memory used by the HatInfo list.
        if (hats != NULL)
        {
            free(hats);
        }
    }
    else
    {
        fprintf(stderr, "Error: No HAT devices found\n");
    }

    return return_val;
}

#endif /* UTILITY_H_ */

