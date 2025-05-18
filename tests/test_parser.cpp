#include <gtest/gtest.h>
#include "parser.h"

TEST(ParserTest, DummyTest) {
    TelemetryPacket packet{123456, 2, -55};
    EXPECT_EQ(packet.sensor_id, 2);
}
