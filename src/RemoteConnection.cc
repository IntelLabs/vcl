/**
 * @file   RemoteConnection.cc
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

#include <fstream>

#ifdef HAVE_S3
    #include <aws/core/utils/StringUtils.h>
    #include <aws/s3/model/PutObjectRequest.h>
    #include <aws/s3/model/GetObjectRequest.h>
#endif 

#include "RemoteConnection.h"

using namespace VCL;

RemoteConnection::RemoteConnection()
{    
    _remote = false;

    #ifdef HAVE_S3
        _config.region = Aws::Utils::StringUtils::to_string("us-west-2");
        _config.requestTimeoutMs = 3000;
        _config.connectTimeoutMs = 3000;
    #endif
}

#ifdef HAVE_S3
RemoteConnection::RemoteConnection(const std::string &region)
{
    _config.region = Aws::Utils::StringUtils::to_string(region);
    _config.requestTimeoutMs = 3000;
    _config.connectTimeoutMs = 3000;
}

RemoteConnection::RemoteConnection(const std::string &region, const std::string &id, 
    const std::string &key)
{
    _config.region = Aws::Utils::StringUtils::to_string(region);
    _config.requestTimeoutMs = 3000;
    _config.connectTimeoutMs = 3000;

    set_s3_credentials(id, key);
}
#endif

RemoteConnection::RemoteConnection(const RemoteConnection &connection)
{    
    _remote = connection._remote;

    #ifdef HAVE_S3
        _options = connection._options;
        _config = connection._config;
        _credentials = connection._credentials;
    #endif
}

void RemoteConnection::operator=(const RemoteConnection &connection)
{    
    _remote = connection._remote;

    #ifdef HAVE_S3
        _options = connection._options;
        _config = connection._config;
        _credentials = connection._credentials;
    #endif
}

RemoteConnection::~RemoteConnection()
{
    if (!_remote)
        end();
}

void RemoteConnection::start()
{
    #ifdef HAVE_S3
        Aws::InitAPI(_options);
    #endif
    _remote = true;
}

void RemoteConnection::end()
{
    #ifdef HAVE_S3
        Aws::ShutdownAPI(_options);
    #endif
    _remote = false;
}

#ifdef HAVE_S3
void RemoteConnection::set_s3_configuration(const std::string &region, 
    const long request_timeout, const long connect_timeout)
{
    _config.region = Aws::Utils::StringUtils::to_string(region);
    _config.requestTimeoutMs = request_timeout;
    _config.connectTimeoutMs = connect_timeout;
}

void RemoteConnection::set_s3_proxy(const std::string &host, const int port)
{
    _config.proxyScheme = Aws::Http::Scheme::HTTPS;
    _config.proxyHost = Aws::Utils::StringUtils::to_string(host);
    _config.proxyPort = port;
}

void RemoteConnection::set_s3_credentials(const std::string &access, const std::string &key)
{
    _credentials.SetAWSAccessKeyId(Aws::Utils::StringUtils::to_string(access));
    _credentials.SetAWSSecretKey(Aws::Utils::StringUtils::to_string(key));
}

std::string RemoteConnection::get_s3_region()
{
    std::stringstream ss;
    ss << _config.region;
    return ss.str();
}

long RemoteConnection::get_s3_result_timeout()
{
    return _config.requestTimeoutMs;
}

long RemoteConnection::get_s3_connect_timeout()
{
    return _config.connectTimeoutMs;
}

std::string RemoteConnection::get_s3_access_id()
{
    std::stringstream ss;
    ss << _credentials.GetAWSAccessKeyId();
    return ss.str();
}

std::string RemoteConnection::get_s3_secret_key()
{
    std::stringstream ss;
    ss << _credentials.GetAWSSecretKey();
    return ss.str();
}

Aws::S3::S3Client RemoteConnection::create_s3_client()
{
    return Aws::S3::S3Client(_credentials, _config);
}
#endif