#include "pch.h"
#include "CppUnitTest.h"
#include "../LightsInstructions.cpp"
#include <iostream>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{
	TEST_CLASS(UnitTests)
	{
	private:

		void RunTestCase(uint16_t T, uint16_t D, uint16_t R, uint16_t S,
			const std::string& problem,
			bool expectedBool,
			const char* expectedCmd = nullptr
		) {
			char returnedCmd[16] = "\0";

			bool returnedBool = solveProblem(problem, T, D, R, S, returnedCmd);

			// Assert that the correct value was returned
			Assert::AreEqual(expectedBool, returnedBool);

			// If true should have been returned
			if (expectedBool)
			{
				// Check that this test case was set up right - if true is expected then a command should be expected too
				Assert::IsNotNull(expectedCmd);

				// Check that the correct value was put into the cmd buffer
				Assert::AreEqual(
					std::string(expectedCmd),
					std::string(returnedCmd)
				);
			}
		}

	public:

		// Equality
		TEST_METHOD(Equality1)
		{
			RunTestCase(0, 0, 0, 0, "=(123,123),xyz", true, "xyz");
		}
		TEST_METHOD(Equality2)
		{
			RunTestCase(0, 0, 0, 0, "=(123,456),xyz", false);
		}

		// Not
		TEST_METHOD(Not1) {
			RunTestCase(0, 0, 0, 0, "!(=(1,2)),xyz", true, "xyz");
		}
		TEST_METHOD(Not2) {
			RunTestCase(0, 0, 0, 0, "!(=(1,1)),xyz", false);
		}

		// Greater than
		TEST_METHOD(GreaterThan1) {
			RunTestCase(0, 0, 0, 0, ">(2,1),xyz", true, "xyz");
		}
		TEST_METHOD(GreaterThan2) {
			RunTestCase(0, 0, 0, 0, ">(1,1),xyz", false);
		}

		// Less than
		TEST_METHOD(LessThan1) {
			RunTestCase(0, 0, 0, 0, "<(1,2),xyz", true, "xyz");
		}
		TEST_METHOD(LessThan2) {
			RunTestCase(0, 0, 0, 0, "<(1,1),xyz", false);
		}

		// Logical and
		TEST_METHOD(LogicalAnd1) {
			RunTestCase(0, 0, 0, 0, "&(=(1,1),=(1,1)),xyz", true, "xyz");
		}
		TEST_METHOD(LogicalAnd2) {
			RunTestCase(0, 0, 0, 0, "&(=(1,1),=(1,2)),xyz", false);
		}
		TEST_METHOD(LogicalAnd3) {
			RunTestCase(0, 0, 0, 0, "&(=(1,2),=(1,2)),xyz", false);
		}
		TEST_METHOD(LogicalAnd4) {
			RunTestCase(0, 0, 0, 0, "&(=(1,2),=(1,2)),xyz", false);
		}

		// Logical or
		TEST_METHOD(LogicalOr1) {
			RunTestCase(0, 0, 0, 0, "|(=(1,1),=(1,1)),xyz", true, "xyz");
		}
		TEST_METHOD(LogicalOr2) {
			RunTestCase(0, 0, 0, 0, "|(=(1,1),=(1,2)),xyz", true, "xyz");
		}
		TEST_METHOD(LogicalOr3) {
			RunTestCase(0, 0, 0, 0, "|(=(1,2),=(1,1)),xyz", true, "xyz");
		}
		TEST_METHOD(LogicalOr4) {
			RunTestCase(0, 0, 0, 0, "|(=(1,2),=(1,2)),xyz", false);
		}

		// Minimum
		TEST_METHOD(Minimum1) {
			RunTestCase(0, 0, 0, 0, "=(1,m(1,2)),xyz", true, "xyz");
		}
		TEST_METHOD(Minimum2) {
			RunTestCase(0, 0, 0, 0, "=(1,m(0,1)),xyz", false);
		}

		// Maximum
		TEST_METHOD(Maximum1) {
			RunTestCase(0, 0, 0, 0, "=(1,M(0,1)),xyz", true, "xyz");
		}
		TEST_METHOD(Maximum2) {
			RunTestCase(0, 0, 0, 0, "=(1,M(1,2)),xyz", false);
		}

		// Time
		TEST_METHOD(Time1) {
			RunTestCase(1, 0, 0, 0, "=(T,1),xyz", true, "xyz");
		}
		TEST_METHOD(Time2) {
			RunTestCase(1, 0, 0, 0, "=(T,2),xyz", false);
		}

		// Day
		TEST_METHOD(Day1) {
			RunTestCase(0, 1, 0, 0, "=(D,1),xyz", true, "xyz");
		}
		TEST_METHOD(Day2) {
			RunTestCase(0, 1, 0, 0, "=(D,2),xyz", false);
		}

		// Sunrise
		TEST_METHOD(Sunrise1) {
			RunTestCase(0, 0, 1, 0, "=(R,1),xyz", true, "xyz");
		}
		TEST_METHOD(Sunrise2) {
			RunTestCase(0, 0, 1, 0, "=(R,2),xyz", false);
		}

		// Sunset
		TEST_METHOD(Sunset1) {
			RunTestCase(0, 0, 0, 1, "=(S,1),xyz", true, "xyz");
		}
		TEST_METHOD(Sunset2) {
			RunTestCase(0, 0, 0, 1, "=(S,2),xyz", false);
		}

		// Weekday
		TEST_METHOD(Weekend1) {
			RunTestCase(0, 0, 0, 0, "=(D,8),xyz", false);
		}
		TEST_METHOD(Weekend2) {
			RunTestCase(0, 1, 0, 0, "=(D,8),xyz", true, "xyz");
		}
		TEST_METHOD(Weekend3) {
			RunTestCase(0, 2, 0, 0, "=(D,8),xyz", true, "xyz");
		}
		TEST_METHOD(Weekend4) {
			RunTestCase(0, 3, 0, 0, "=(D,8),xyz", true, "xyz");
		}
		TEST_METHOD(Weekend5) {
			RunTestCase(0, 4, 0, 0, "=(D,8),xyz", true, "xyz");
		}
		TEST_METHOD(Weekend6) {
			RunTestCase(0, 5, 0, 0, "=(D,8),xyz", true, "xyz");
		}
		TEST_METHOD(Weekend7) {
			RunTestCase(0, 6, 0, 0, "=(D,8),xyz", false);
		}

		// Weekend
		TEST_METHOD(Weekday1) {
			RunTestCase(0, 0, 0, 0, "=(D,9),xyz", true, "xyz");
		}
		TEST_METHOD(Weekday2) {
			RunTestCase(0, 1, 0, 0, "=(D,9),xyz", false);
		}
		TEST_METHOD(Weekday3) {
			RunTestCase(0, 2, 0, 0, "=(D,9),xyz", false);
		}
		TEST_METHOD(Weekday4) {
			RunTestCase(0, 3, 0, 0, "=(D,9),xyz", false);
		}
		TEST_METHOD(Weekday5) {
			RunTestCase(0, 4, 0, 0, "=(D,9),xyz", false);
		}
		TEST_METHOD(Weekday6) {
			RunTestCase(0, 5, 0, 0, "=(D,9),xyz", false);
		}
		TEST_METHOD(Weekday7) {
			RunTestCase(0, 6, 0, 0, "=(D,9),xyz", true, "xyz");
		}

		// Errors
		TEST_METHOD(Error1) {
			RunTestCase(0, 0, 0, 0, "=(,xyz", true, "error");
		}
		TEST_METHOD(Error2) {
			RunTestCase(0, 0, 0, 0, "(1,1),xyz", true, "error");
		}
		TEST_METHOD(Error3) {
			RunTestCase(0, 0, 0, 0, "=(!(=(1,1)),1),xyz", true, "error");
		}
		TEST_METHOD(Error4) {
			RunTestCase(0, 0, 0, 0, "!(1),xyz", true, "error");
		}
	};
}
