#include <gtest/gtest.h>
#include "foo.hpp"

TEST(FooLib, FooTest)
{
	EXPECT_EQ(foo(2), 4);
}

#ifdef FEATURE_BAR
TEST(FooLib, BarTest)
{
	EXPECT_TRUE(bar(2));
	EXPECT_FALSE(bar(3));
}
#endif

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}