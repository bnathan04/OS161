/*
 * console.c
 *
 * Tests basic functionalities within stdio.h and err.h and tests the read and
 * write syscalls with bad input for assignment 0
 *
 * This should work once read() and write() system calls are complete. However,
 * you should try testbin/printf first.
 *
 * Note - we will not test read() with buflen greater than 1. However, you 
 *        should set errno to ENOSYS if you did not implement a solution that 
 *        is capable of reading more than 1 character at once.
 *
 *
 * Kuei Sun <kuei.sun@mail.utoronto.ca>
 *
 * University of Toronto, 2016
 */
 
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <err.h>

#define TEST(expr, val) do { \
        printf("Running test case %d... ", ++testno); \
        ret = expr;     \
        if ( ret >= 0 ) { \
                printf("failed. Expecting negative return value, " \
                "got %d.\n", ret); \
        } else if ( errno != val ) { \
                printf("failed. Expecting error number %d, got %d.\n", \
                val, ret); \
        } else { \
                printf("passed.\n"); \
        } \
} while(0)

static void test_bad_rw(void)
{
    int ret;
    int testno = 0;
    char * badbuf  = (char *)0xbadbeef;
    char * badbuf2 = (char *)0x0;
    char * badbuf3 = (char *)0xdeadbeef;
    char buf[16]; 


    // stdin is read-only
    TEST(write(STDIN_FILENO, "c", 1), EBADF);

    // fd 5 is not opened (and cannot be opened until we implement it...)
    TEST(write(5, "hello", 5), EBADF);

    // buf points to invalid address
    TEST(write(STDOUT_FILENO, badbuf, 10), EFAULT); 
    TEST(write(STDERR_FILENO, badbuf2, 2), EFAULT); 
    TEST(write(STDERR_FILENO, badbuf3, 7), EFAULT);

    // stdout and stderr are write-only
    TEST(read(STDOUT_FILENO, buf, 1), EBADF);
    TEST(read(STDERR_FILENO, buf, 1), EBADF);

    // fd 9 not opened
    TEST(read(9, buf, 1), EBADF);

    printf("Press any key 3 times in the next set of tests.\n");

    // buf points to invalid address
    TEST(read(STDIN_FILENO, badbuf,  1), EFAULT);
    TEST(read(STDIN_FILENO, badbuf2, 1), EFAULT);
    TEST(read(STDIN_FILENO, badbuf3, 1), EFAULT);
}



void printstring(const char * str) 
{
    size_t len = strlen(str);
    int actual = write(STDOUT_FILENO, str, len);
    if ( actual < 0 )
        err(1, "write");
    if ( actual != (int)len )
        warnx("return value of write does not equal to input length\n");
}

static void test_rw(void) 
{
    char ch;
    
    printf("1. writing to stdout... ");
    printstring("hello world!\n");
 
    printf("2. writing to stderr... ");
    warnx("false warning!");
    
    printf("3. reading from stdin...\n");
    printf("Press Enter: ");
    ch = getchar();
    
    if ( ch != '\n' && ch != '\r' ) {
        printf("fail to read newline or carriage return from stdin "
               "(got 0x%x)\n", (unsigned)ch);
    }
    else {
        printf("passed.\n");
    }
    
    printf("Press 6: ");
    if ( (ch = getchar()) != '6' ) {
        printf("fail to read the 6 from stdin (got 0x%x)\n", (unsigned)ch);
    }
    else {
        printf("passed.\n");
    }
}

int main(void) {
    test_rw();
    test_bad_rw();
	return 0;
}   
