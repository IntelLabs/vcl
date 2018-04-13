/**
 * @file   RemoteConnection.h
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
 * @section DESCRIPTION
 *
 * This file declares the C++ API for RemoteConnection, which allows users to 
 *  connect to different file systems. At the moment, S3 is enabled. 
 */

#pragma once

#include "Exception.h"

#ifdef HAVE_S3
    #include <aws/core/Aws.h>
    #include <aws/s3/S3Client.h>
    #include <aws/core/client/ClientConfiguration.h>
    #include <aws/core/auth/AWSCredentialsProvider.h>
#endif

namespace VCL {

    class RemoteConnection{
    public:
        RemoteConnection();
        #ifdef HAVE_S3
            RemoteConnection(const std::string &region);
            RemoteConnection(const std::string &region, const std::string &id, 
                const std::string &key);
        #endif
        RemoteConnection(const RemoteConnection &connection);
        void operator=(const RemoteConnection &connection);

        ~RemoteConnection();

        void start();
        void end();

        bool connected() { return _remote; };

        #ifdef HAVE_S3
            void set_s3_configuration(const std::string &region, 
                const long result_timeout=3000, const long connect_timeout=3000);
            void set_s3_proxy(const std::string &host, const int port);
            void set_s3_credentials(const std::string &access, const std::string &key);

            std::string get_s3_region();
            long get_s3_result_timeout();
            long get_s3_connect_timeout();
            std::string get_s3_access_id();
            std::string get_s3_secret_key();

            Aws::S3::S3Client create_s3_client();
        #endif

    private:
        bool _remote;

        #ifdef HAVE_S3
            Aws::Auth::AWSCredentials _credentials;
            Aws::SDKOptions _options;
            Aws::Client::ClientConfiguration _config;
        #endif
    };
}
