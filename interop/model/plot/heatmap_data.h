/** Encapsulates all the data for a heat map
 *
 *  @file
 *  @date 5/6/16
 *  @version 1.0
 *  @copyright GNU Public License.
 */
#pragma once

#include "interop/util/assert.h"
#include "interop/util/exception.h"
#include "interop/model/plot/series.h"
#include "interop/model/plot/axes.h"
#include "interop/model/plot/chart_data.h"

namespace illumina { namespace interop { namespace model { namespace plot
{

    /** Encapsulates all data for a heatmap
     */
    class heatmap_data : public chart_data
    {

    public:
        /** Constructor */
        heatmap_data() : m_data(0), m_num_columns(0), m_num_rows(0), m_free(false)
        { }
        /** Constructor
         *
         * @param data use the given buffer to back the heat map
         * @param rows number of rows in the heat map
         * @param cols number of columns in the heat map
         */
        heatmap_data(float* data, const size_t rows, const size_t cols) :
                m_data(data), m_num_columns(cols), m_num_rows(rows), m_free(false)
        { }
        /** Destructor */
        virtual ~heatmap_data()
        {
            clear();
        }

    public:
        /** Resize the heat map to the given number of rows and columns
         *
         * @param data use the given buffer to back the heat map
         * @param rows number of rows
         * @param cols number of columns
         */
        void set_buffer(float* data, const size_t rows, const size_t cols)
        {
            if(m_free) delete[] m_data;
            m_data = data;
            m_num_columns = cols;
            m_num_rows = rows;
            m_free=false;
        }
        /** Resize the heat map to the given number of rows and columns
         *
         * @param rows number of rows
         * @param cols number of columns
         */
        void resize(const size_t rows, const size_t cols)
        {
            if(rows != m_num_rows && cols != m_num_columns)
            {
                if (m_free) delete[] m_data;
                m_data = new float[cols * rows];
                std::memset(reinterpret_cast<char*>(m_data), 0, sizeof(float)*rows*cols);
                m_num_columns = cols;
                m_num_rows = rows;
                m_free = true;
            }
        }

        /** Get value at given row and column
         *
         * TODO: This should thrown an exception if wrong
         *
         * @param row row index
         * @param col column index
         * @return value
         */
        float at(const size_t row, const size_t col) const throw(model::index_out_of_bounds_exception)
        {
            if (row >= m_num_rows)
                INTEROP_THROW(model::index_out_of_bounds_exception, "Row index out of bounds");
            if (col >= m_num_columns)
                INTEROP_THROW(model::index_out_of_bounds_exception, "Column index out of bounds");
            const size_t idx = index_of(row, col);
            INTEROP_ASSERT(idx < m_num_rows*m_num_columns);
            INTEROP_ASSERT(m_data != 0);
            return m_data[idx];
        }

        /** Get value at given row and column
         *
         * TODO: This should thrown an exception if wrong
         *
         * @param row row index
         * @param col column index
         * @return value
         */
        float &operator()(const size_t row, const size_t col) throw(model::index_out_of_bounds_exception)
        {
            if (row >= m_num_rows)
                INTEROP_THROW(model::index_out_of_bounds_exception, "Row index out of bounds");
            if (col >= m_num_columns)
                INTEROP_THROW(model::index_out_of_bounds_exception, "Column index out of bounds");
            const size_t idx = index_of(row, col);
            INTEROP_ASSERT(idx < m_num_rows*m_num_columns);
            INTEROP_ASSERT(m_data != 0);
            return m_data[idx];
        }

        /** Get value at given row and column
         *
         * TODO: This should thrown an exception if wrong
         *
         * @param row row index
         * @param col column index
         * @return value
         */
        float operator()(const size_t row, const size_t col) const throw(model::index_out_of_bounds_exception)
        {
            if (row >= m_num_rows)
                INTEROP_THROW(model::index_out_of_bounds_exception, "Row index out of bounds");
            if (col >= m_num_columns)
                INTEROP_THROW(model::index_out_of_bounds_exception, "Column index out of bounds");
            const size_t idx = index_of(row, col);
            INTEROP_ASSERT(idx < m_num_rows*m_num_columns);
            INTEROP_ASSERT(m_data != 0);
            return m_data[idx];
        }

        /** Get number of rows
         *
         * @return number of rows
         */
        size_t row_count() const
        {
            return m_num_rows;
        }

        /** Get number of columns
         *
         * @return number of columns
         */
        size_t column_count() const
        {
            return m_num_columns;
        }

        /** Get number of rows * columns
         *
         * @return number of rows * columns
         */
        size_t length() const
        {
            return m_num_columns * m_num_rows;
        }

        /** Clear the data
         */
        void clear()
        {
            if(m_free)
            {
                delete[] m_data;
                m_data=0;
                m_free = false;
            }
            m_num_columns = 0;
            m_num_rows = 0;
        }

    protected:
        /** Get the index of the row and column in the array
         *
         * @param row row index
         * @param col column index
         * @return index in array
         */
        size_t index_of(const size_t row, const size_t col) const
        {
            return row * m_num_columns + col;
        }

    private:
        float* m_data;
        size_t m_num_columns;
        size_t m_num_rows;
        bool m_free;
    };

}}}}
