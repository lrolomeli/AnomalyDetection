#include <stdio.h>
#include <time.h>
//
#include "ff.h"
#include "util.h"  // calculate_checksum
//
#include "rtc.h"

static time_t epochtime;
// Start on Friday 5th of June 2020 15:45:00
static const datetime_t custom_datetime = {
        .year  = 2024,
        .month = 9,
        .day   = 24,
        .dotw  = 2, // 0 is Sunday, so 5 is Friday
        .hour  = 11,
        .min   = 31,
        .sec   = 59
};

// Make an attempt to save a recent time stamp across reset:
typedef struct rtc_save {
    uint32_t signature;
    datetime_t datetime;
    uint32_t checksum;  // last, not included in checksum
} rtc_save_t;

static rtc_save_t rtc_save __attribute__((section(".uninitialized_data")));

static void update_epochtime() {
    bool rc = rtc_get_datetime(&rtc_save.datetime);
    if (rc) {
        rtc_save.signature = 0xBABEBABE;
        struct tm timeinfo = {
            .tm_sec = rtc_save.datetime
                          .sec, /* Seconds.	[0-60] (1 leap second) */
            .tm_min = rtc_save.datetime.min,          /* Minutes.	[0-59] */
            .tm_hour = rtc_save.datetime.hour,        /* Hours.	[0-23] */
            .tm_mday = rtc_save.datetime.day,         /* Day.		[1-31] */
            .tm_mon = rtc_save.datetime.month - 1,    /* Month.	[0-11] */
            .tm_year = rtc_save.datetime.year - 1900, /* Year	- 1900.  */
            .tm_wday = 0,                             /* Day of week.	[0-6] */
            .tm_yday = 0,                             /* Days in year.[0-365]	*/
            .tm_isdst = -1                            /* DST.		[-1/0/1]*/
        };
        rtc_save.checksum = calculate_checksum((uint32_t *)&rtc_save,
                                               offsetof(rtc_save_t, checksum));
        epochtime = mktime(&timeinfo);
        rtc_save.datetime.dotw = timeinfo.tm_wday;
        // configASSERT(-1 != epochtime);
    }
}

time_t time(time_t *pxTime) {
    update_epochtime();
    if (pxTime) {
        *pxTime = epochtime;
    }
    return epochtime;
}

void time_init() {
    // Start the RTC
    rtc_init();
    rtc_set_datetime(&custom_datetime);
}

// Function to get the current date and time from the RTC
void get_formatted_datetime(char *buffer, char type) {

    // Structure to store date and time information
    datetime_t t = {0, 0, 0, 0, 0, 0, 0};
    // Assuming the RTC is initialized, retrieve the current time from it
    // You will need to replace this with your specific RTC function to read time
    rtc_get_datetime(&t);

    // Format the date and time in the desired format: dd-mm-yyyy-hh-mm-ss.csv
    snprintf(buffer, FORMATTED_BUFSIZE, "%c-%02d-%02d-%04d-%02d-%02d-%02d.dat", 
             type,
             t.day,    // Day of the month (dd)
             t.month, // Month (tm_mon starts at 0, so we add 1)
             t.year, // Year (tm_year is years since 1900)
             t.hour,    // Hours (hh)
             t.min,     // Minutes (mm)
             t.sec      // Seconds (ss)
    );
}

void time_init_deprecated() {
    rtc_init();
    datetime_t t = {0, 0, 0, 0, 0, 0, 0};
    rtc_get_datetime(&t);
    if (!t.year && rtc_save.datetime.year) {
        uint32_t xor_checksum = calculate_checksum(
            (uint32_t *)&rtc_save, offsetof(rtc_save_t, checksum));
        if (rtc_save.signature == 0xBABEBABE &&
            rtc_save.checksum == xor_checksum) {
            // Set rtc
            rtc_set_datetime(&rtc_save.datetime);
        }
    }
}

// Called by FatFs:
DWORD get_fattime(void) {
    datetime_t t = {0, 0, 0, 0, 0, 0, 0};
    bool rc = rtc_get_datetime(&t);
    if (!rc) return 0;

    DWORD fattime = 0;
    // bit31:25
    // Year origin from the 1980 (0..127, e.g. 37 for 2017)
    uint8_t yr = t.year - 1980;
    fattime |= (0b01111111 & yr) << 25;
    // bit24:21
    // Month (1..12)
    uint8_t mo = t.month;
    fattime |= (0b00001111 & mo) << 21;
    // bit20:16
    // Day of the month (1..31)
    uint8_t da = t.day;
    fattime |= (0b00011111 & da) << 16;
    // bit15:11
    // Hour (0..23)
    uint8_t hr = t.hour;
    fattime |= (0b00011111 & hr) << 11;
    // bit10:5
    // Minute (0..59)
    uint8_t mi = t.min;
    fattime |= (0b00111111 & mi) << 5;
    // bit4:0
    // Second / 2 (0..29, e.g. 25 for 50)
    uint8_t sd = t.sec / 2;
    fattime |= (0b00011111 & sd);
    return fattime;
}
