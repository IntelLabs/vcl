/**
 * @file   main_test.cc
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "gtest/gtest.h"
#include "Image.h"

#ifdef HAVE_S3
	#include <aws/core/utils/memory/stl/AWSString.h>
	#include <aws/core/utils/logging/DefaultLogSystem.h>
	#include <aws/core/utils/logging/AWSLogging.h>
#endif

int main(int argc, char** argv) 
{
	// Aws::Utils::Logging::InitializeAWSLogging(
 //    Aws::MakeShared<Aws::Utils::Logging::DefaultLogSystem>(
 //        "RunUnitTests", Aws::Utils::Logging::LogLevel::Trace, "aws_sdk_"));
	::testing::InitGoogleTest(&argc, argv);
    int exitCode = RUN_ALL_TESTS();
    // Aws::Utils::Logging::ShutdownAWSLogging();
    return exitCode;
}
