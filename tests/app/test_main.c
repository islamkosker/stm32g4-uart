#include "unity.h"

void test_protocol(void);

void setUp(void) {}
void tearDown(void) {}

int main(void)
{
    UNITY_BEGIN();
    test_protocol();
    return UNITY_END();
}
