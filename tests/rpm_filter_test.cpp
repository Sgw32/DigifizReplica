#define CATCH_CONFIG_MAIN
#include "catch.hpp"
extern "C" {
#include "rpm_filter.h"
}

TEST_CASE("init_buffer initializes data") {
    CircularBuffer b;
    b.data[0] = 123;
    init_buffer(&b);
    for(int i=0;i<RPM_WINDOW_SIZE;i++) {
        REQUIRE(b.data[i] == 0);
    }
    REQUIRE(b.index == 0);
}

TEST_CASE("insert_buffer updates index and data") {
    CircularBuffer b;
    init_buffer(&b);
    insert_buffer(&b, 1);
    REQUIRE(b.data[0] == 1);
    REQUIRE(b.index == 1);
    for(int i=0;i<RPM_WINDOW_SIZE-1;i++) {
        insert_buffer(&b, i+2);
    }
    REQUIRE(b.index == 0);
    insert_buffer(&b, 99);
    REQUIRE(b.data[b.index==0?RPM_WINDOW_SIZE-1:b.index-1] == 99);
}

TEST_CASE("median_filter computes median") {
    CircularBuffer b;
    init_buffer(&b);
    for(int i=1;i<=RPM_WINDOW_SIZE;i++) {
        median_filter(&b, i);
    }
    // After inserting 1..8, median should be 5
    REQUIRE(find_median(&b) == 5);
    int m = median_filter(&b, 9); // window now 9,2..8
    REQUIRE(m == 6);
}
