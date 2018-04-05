/**
 * @file   TDBObject.h
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
 * This file declares the C++ API for TDBObject
 */

#pragma once

#include <iostream>

#include <string>
#include <vector>
#include <stdlib.h>

#include <tiledb/tiledb.h>
#include "Exception.h"
#include "utils.h"

namespace VCL {

    /*  *********************** */
    /*          MACRO           */
    /*  *********************** */
    #define Error_Check(answer, ...) { TDBAssert(answer, ##__VA_ARGS__, __FILE__, __LINE__); }
    inline void TDBAssert(int tdb_return, tiledb_ctx_t* ctx, std::string message, const char* file, int line)
    {
        if (tdb_return == TILEDB_ERR) {
            tiledb_error_t* err = NULL;
            tiledb_ctx_get_last_error(ctx, &err);
            const char* msg;
            tiledb_error_message(err, &msg);
            // printf("%s\n", msg);
            std::string full_err_msg = message + "\n" + std::string(msg);
            throw VCLException(TileDBError, full_err_msg);
        }
    }

    class TDBObject {

    /*  *********************** */
    /*        VARIABLES         */
    /*  *********************** */
    protected:
        // Path variables
        std::string _group;
        std::string _name;

        // Dimensions (defines the type of TDBObject, should be set in inherited class)
        int _num_dimensions;
        std::vector<std::string> _dimension_names;
        std::vector<uint64_t> _dimension_values;

        // Attributes (number of values in a cell)
        int _num_attributes;
        std::vector<const char*> _attributes;

        // Compression type
        CompressionType _compressed;
        int _min_tile_dimension;

        // TileDB variables
        std::vector<uint64_t> _array_dimension;
        std::vector<uint64_t> _tile_dimension;
        tiledb_ctx_t* _ctx;

    public:

    /*  *********************** */
    /*        CONSTRUCTORS      */
    /*  *********************** */
        /**
         *  Creates a empty TDBObject
         */
        TDBObject();

        /**
         *  Creates a TDBObject from an object id
         *
         *  @param object_id  The path of the TDBObject
         */
        TDBObject(const std::string &object_id);

        /**
         *  Creates a TDBObject from an existing TDBObject
         *
         *  @param tdb  A reference to an existing TDBObject
         */
        TDBObject(const TDBObject &tdb);

        /**
         *  Sets a TDBObject equal to another TDBObject
         *
         *  @param tdb  A reference to an existing TDBObject
         *  @return The current TDBObject
         */
        TDBObject& operator=(const TDBObject &tdb);

        /**
         *  TDBObject destructor
         */
        ~TDBObject();


    /*  *********************** */
    /*        GET FUNCTIONS     */
    /*  *********************** */
        /**
         *  Gets the path to the TDBObject
         *
         *  @return The string containing the full path to the TDBObject
         */
        std::string get_object_id() const;


    /*  *********************** */
    /*        SET FUNCTIONS     */
    /*  *********************** */
        /**
         *  Sets the number of dimensions in the TDBObject, specific
         *    to the type of TDBObject it will be (Vector objects have
         *    one dimension, Image objects have two dimensions,
         *    Volume objects have 3)
         *
         *  @param num_dimensions  The number of dimensions
         */
        void set_num_dimensions(int num_dimensions);

        /**
         *  Sets the names of the dimensions in the TDBObject
         *
         *  @param dimensions  A vector of strings that define the
         *    names of the dimensions
         */
        void set_dimensions(const std::vector<std::string> &dimensions);

        /**
         *  Sets the values of the dimensions in the TDBObject
         *
         *  @param dimensions  A vector of integers that define the
         *    value of each dimension
         */
        void set_dimension_values(const std::vector<uint64_t> &dimensions);

        /**
         *  Sets the minimum tile dimension
         *
         *  @param min  The minimum number of tiles per dimension
         */
        void set_minimum(int dimension);

        /**
         *  Implemented by the specific TDBObject classes, sets
         *    the names of the dimensions to standard defaults
         */
        virtual void set_default_dimensions() = 0;

        /**
         *  Sets the number of attributes in the TDBObject, which defines
         *    how the array is stored. Default is usually one
         *
         *  @param num_attributes  The number of attributes
         */
        void set_num_attributes(int num_attributes);
        /**
         *  Sets the names of attributes in the TDBObject
         *
         *  @param attributes  A vector of strings that define the
         *    names of the attributes
         */
        void set_attributes(const std::vector<std::string> &attributes);

        /**
         *  Implemented by the specific TDBObject classes, sets
         *    the names of the attributes to standard defaults
         */
        virtual void set_default_attributes() = 0;

        /**
         *  Sets the type of compression to be used when compressing
         *    the TDBObject
         *
         *  @param comp  The compression type
         *  @see Image.h for details on CompressionType
         */
        void set_compression(CompressionType comp);


    /*  *********************** */
    /*  TDBOBJECT INTERACTION   */
    /*  *********************** */
        /**
         *  Implemented by the specific TDBObject class, writes the data in
         *    the TDBObject to the given object id
         *
         *  @param object_id  The object id where the data is to be written
         *  @param  metadata  A flag indicating whether the metadata
         *    should be stored in TileDB or not. Defaults to true
         */
        virtual void write(const std::string &object_id, bool metadata = true) = 0;

        /**
         *  Implemented by the specific TDBObject class, reads the data from
         *    the TDBObject specified by the path variables
         */
        virtual void read() = 0;

        /**
         *  Deletes the object from TileDB
         */
        void delete_object();



    protected:
    /*  *********************** */
    /*        GET FUNCTIONS     */
    /*  *********************** */
        /**
         *  Gets the location of the last / in an object id
         *
         *  @param  object_id  A string
         *  @return The location of the last / in the given string
         */
        size_t get_path_delimiter(const std::string &object_id) const;

        /**
         *  Gets the parent directory of a file (the TileDB group)
         *    and tries to create the directory if it does not exist
         *
         *  @param  filename  The full path of the file
         *  @param  pos  The location of the last / in the filename
         *  @return The name of the TileDB group
         */
        std::string get_group(const std::string &filename, size_t pos) const;

        /**
         *  Gets the name of a file (the TileDB array)
         *
         *  @param  filename  The full path of the file
         *  @param  pos  The location of the last / in the filename
         *  @return The name of the TileDB array
         */
        std::string get_name(const std::string &filename, size_t pos) const;


    /*  *********************** */
    /*        SET FUNCTIONS     */
    /*  *********************** */
        /**
         *  Sets the member variables of one TDBObject equal to another
         *
         *  @param  tdb  The TDBOjbect to set the current TDBObject's
         *    variables equal to
         */
        void set_equal(const TDBObject &tdb);

        /**
         *  Determines the TileDB schema variables and sets the
         *    schema for writing a dense TileDB array
         *
         *  @param  cell_val_num  The number of values per cell in the array
         *  @param  object_id  The full path to the TileDB array
         */
        void set_schema_dense(int cell_val_num, const std::string &object_id);
        
        /**
         *  Determines the TileDB schema variables and sets the
         *    schema for writing a sparse TileDB array
         *
         *  @param  cell_val_num  The number of values per cell in the array
         *  @param  object_id  The full path to the TileDB array
         */
        void set_schema_sparse(int cell_val_num, const std::string &object_id);
        
        /**
         *  Sets the TDBObject values from an array schema
         *
         *  @param  object_id  The full path to the TileDB array
         */
        void set_from_schema(const std::string &object_id);

    /*  *********************** */
    /*   METADATA INTERACTION   */
    /*  *********************** */
        /**
         *  Writes the TDBObject metadata
         *
         *  @param  metadata  The full path to the TileDB array metadata
         *  @param  keys  A vector containing the metadata keys
         *  @param  values  A vector containing the metadata values
         */
        void write_metadata(const std::string &metadata, const std::vector<std::string> &keys, 
            const std::vector<uint64_t> &values);

        /**
         *  Implemented by the specific TDBObject class, reads the
         *    metadata associated with the TDBObject
         */
        virtual void read_metadata() = 0;

    private:
        /**
         *  Sets the TileDB type of the attribute values, currently
         *    all are unsigned characters
         *
         *  @param  types  An array to be filled with the attribute
         *    value types
         */
        void set_types(int* types);

        /**
         *  Finds the greatest factor of a number
         *
         *  @param a  The number to factor
         *  @return  The greatest factor of a
         */
        int greatest_factor(int a);


        /**
         *  Resets the arrays that are members of this class
         */
        void reset_arrays();

        /**
         *  Sets the TileDB schema dimensions to the appropriate values
         *
         *  @param array_schema  The TileDB array schema
         */
        void set_schema_dimensions(tiledb_array_schema_t* array_schema);

        /**
         *  Sets the TileDB schema attributes to the appropriate values
         *
         *  @param array_schema  The TileDB array schema
         *  @param cell_val_num  The number of values per cell
         */
        void set_schema_attributes(tiledb_array_schema_t* array_schema, int cell_val_num);

        /**
         *  Sets the TileDB schema 
         *
         *  @param cell_val_num  The number of values per cell
         *  @param object_id  The full path to the TileDB array
         *  @param array_schema  The TileDB array schema
         */
        void set_schema(int cell_val_num, const std::string &object_id, tiledb_array_schema_t* array_schema);

        /**
         *  Converts the VCL CompressionType to TileDB compression
         */
        tiledb_compressor_t convert_to_tiledb();

        /**
         *  Determines the size of the TDBObject array as well as
         *    the size of the tiles. Currently tiles have the same
         *    length in all dimensions, and the minimum number of
         *    tiles is 100
         */
        void find_tile_extents();
    };
};
