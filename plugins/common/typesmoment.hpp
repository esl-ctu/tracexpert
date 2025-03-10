#ifndef TYPESMOMENT_HPP
#define TYPESMOMENT_HPP

#include <algorithm>
#include <memory>
#include <utility>
#include <QtGlobal>

namespace SICAK {

    /* Basic data types */

    template <class T>
    class Vector {

    public:

        /// Constructs an empty Vector with no elements. Needs to be initialized first (init).
        Vector() : m_data(nullptr), m_length(0), m_capacity(0) {}
        /// Constructs a Vector with 'length' elements
        Vector(size_t length) : m_data(nullptr), m_length(0), m_capacity(0) {
            (*this).init(length);
        }
        /// Constructs a Vector with 'length' elements and fills it with 'initVal'
        Vector(size_t length, T initVal) : m_data(nullptr), m_length(0), m_capacity(0) {
            (*this).init(length);
            (*this).fill(initVal);
        }

        /// Move constructor
        Vector(Vector&& other) : m_data(std::move(other.m_data)), m_length(other.m_length) {  }
        /// Move assignment operator
        Vector& operator=(Vector&& other) {
            m_data = std::move(other.m_data);
            m_length = other.m_length;
            return (*this);
        }

        /// Empty destructor
        virtual ~Vector() {}

        virtual size_t length() const { return m_length; }

        virtual size_t size() const { return m_length * sizeof(T); }

        virtual void   init(size_t length) {

            // realloc if asking for more than I can take
            if(length > m_capacity){

                try {

                    m_data.reset(new T[length]);
                    m_capacity = length;

                } catch (std::bad_alloc & e) {
                    (void)e; // supress MSVC warnings about an unused local variable
                    qFatal("Memory allocation failed");
                }

            }

            m_length = length;  //< set the length the user have asked for (array is at least this big)

        }

        virtual void   init(size_t length, T initVal) {
            (*this).init(length);
            (*this).fill(initVal);
        }

        virtual void    fill(T val) {
            std::fill_n(m_data.get(), m_length, val);
        }

        virtual T *     data() { return m_data.get(); }

        virtual const T *     data() const { return m_data.get(); }

        T & operator()       (size_t index) { return m_data[index]; }

        const   T & operator()       (size_t index) const { return m_data[index]; }

    protected:

        /// Unique_ptr holding the allocated space
        std::unique_ptr<T[]> m_data;
        /// The number of elements in the vector
        size_t m_length;
        size_t m_capacity;

    };


    template <class T>
    class Matrix {

    public:

        /// Constructs an empty Matrix with no elements. Needs to be initialized first (init).
        Matrix() : m_vector(), m_cols(0), m_rows(0) {}
        /// Constructs a Matrix with 'cols' * 'rows' elements
        Matrix(size_t cols, size_t rows) : m_vector(cols * rows), m_cols(cols), m_rows(rows) {}
        /// Constructs a Matrix with 'cols' * 'rows' elements and fills it with 'initVal'
        Matrix(size_t cols, size_t rows, T initVal) : m_vector(cols * rows, initVal), m_cols(cols), m_rows(rows) {}

        /// Move constructor
        Matrix(Matrix&& other) : m_vector(std::move(other.m_vector)), m_cols(other.m_cols), m_rows(other.m_rows) {}
        /// Move assignment operator
        Matrix& operator=(Matrix&& other) {
            m_vector = std::move(other.m_vector);
            m_cols = other.m_cols;
            m_rows = other.m_rows;
            return (*this);
        }

        /// Empty destructor
        virtual ~Matrix() {}

        virtual size_t cols() const { return m_cols; }
        virtual size_t rows() const { return m_rows; }

        virtual void   init(size_t cols, size_t rows) {
            m_vector.init(cols * rows);
            m_cols = cols;
            m_rows = rows;
        }
        virtual void   init(size_t cols, size_t rows, T initVal) {
            m_vector.init(cols * rows, initVal);
            m_cols = cols;
            m_rows = rows;
        }

        virtual void   shrinkRows(size_t rows) {
            if(rows > m_rows) qFatal("Cannot shrink Matrix to a larger size!");
            m_vector.init(m_cols * rows); // (rows <= m_rows) -> (Vector::m_capacity >= m_cols * rows) -> no memory reallocation happens, only upper bound is lowered
            m_rows = rows;
            // Matrix now appears "less tall"... enlargening it back to the previous size would actually give back the original matrix, but we cant guarantee that generally
        }

        virtual T *    data() { return m_vector.data(); }
        virtual const T *    data() const { return m_vector.data(); }

        virtual size_t length() const { return m_vector.length(); }
        virtual size_t size() const { return m_vector.size(); }

        virtual     void fill(T val) {
            m_vector.fill(val);
        }

        virtual         T & operator()       (size_t col, size_t row) { return m_vector(row * m_cols + col); }
        virtual const   T & operator()       (size_t col, size_t row) const { return m_vector(row * m_cols + col); }

    protected:

        /// A Matrix is composed using a large Vector
        Vector<T> m_vector;
        /// Number of columns
        size_t m_cols;
        /// Number of rows
        size_t m_rows;

    };

    /* Power analysis aliases */

    template <class T>
    class PowerTraces : public Matrix<T> {

    public:
        /// Constructs an empty Matrix with no elements. Needs to be initialized first (init).
        PowerTraces() {}
        /// Constructs a Matrix with 'samplesPerTrace' * 'noOfTraces' elements
        PowerTraces(size_t samplesPerTrace, size_t noOfTraces) : Matrix<T>(samplesPerTrace, noOfTraces) {}
        /// Constructs a Matrix with 'samplesPerTrace' * 'noOfTraces' elements and fills it with 'initVal'
        PowerTraces(size_t samplesPerTrace, size_t noOfTraces, T initVal) : Matrix<T>(samplesPerTrace, noOfTraces, initVal) {}

        /// Move constructor
        PowerTraces(PowerTraces&& other) : Matrix<T>(std::move(other)) { }
        /// Move assignment operator
        PowerTraces& operator=(PowerTraces&& other) { return static_cast<PowerTraces&>(Matrix<T>::operator=(std::move(other))); }

        /// Empty destructor
        virtual ~PowerTraces() {}

        virtual void   init(size_t samplesPerTrace, size_t noOfTraces) { return Matrix<T>::init(samplesPerTrace, noOfTraces); }

        /// Returns number of samples per trace
        virtual size_t samplesPerTrace() const { return (*this).cols(); }
        /// Returns number of power traces
        virtual size_t noOfTraces() const { return (*this).rows(); }

        virtual         T & operator()       (size_t sample, size_t trace) { return Matrix<T>::operator()(sample, trace); }
        virtual const   T & operator()       (size_t sample, size_t trace) const { return Matrix<T>::operator()(sample, trace); }
    };


    template <class T>
    class PowerPredictions : public Matrix<T> {

    public:
        /// Constructs an empty Matrix with no elements. Needs to be initialized first (init).
        PowerPredictions() {}
        /// Constructs a Matrix with 'noOfCandidates' * 'noOfTraces' elements
        PowerPredictions(size_t noOfCandidates, size_t noOfTraces) : Matrix<T>(noOfCandidates, noOfTraces) {}
        /// Constructs a Matrix with 'noOfCandidates' * 'noOfTraces' elements and fills it with 'initVal'
        PowerPredictions(size_t noOfCandidates, size_t noOfTraces, T initVal) : Matrix<T>(noOfCandidates, noOfTraces, initVal) {}

        /// Move constructor
        PowerPredictions(PowerPredictions&& other) : Matrix<T>(std::move(other)) { }
        /// Move assignment operator
        PowerPredictions& operator=(PowerPredictions&& other) { return static_cast<PowerPredictions&>(Matrix<T>::operator=(std::move(other))); }

        /// Empty destructor
        ~PowerPredictions() {}

        virtual void   init(size_t noOfCandidates, size_t noOfTraces) { return Matrix<T>::init(noOfCandidates, noOfTraces); }

        /// Returns number of key candidates per power prediction
        virtual size_t noOfCandidates() const { return (*this).cols(); }
        /// Returns number of power predictions
        virtual size_t noOfTraces() const { return (*this).rows(); }

        virtual         T & operator()       (size_t keyCandidate, size_t trace) { return Matrix<T>::operator()(keyCandidate, trace); }
        virtual const   T & operator()       (size_t keyCandidate, size_t trace) const { return Matrix<T>::operator()(keyCandidate, trace); }
    };

    /* Statistical context */

    template <class T>
    class Moments2DContext {

    public:

        /// Constructs an empty context, needs to be initialized first (init)
        Moments2DContext():
            m_p1Width(0), m_p2Width(0), m_p1Card(0), m_p2Card(0),
            m_p1MOrder(0), m_p2MOrder(0), m_p1CSOrder(0), m_p2CSOrder(0), m_p12ACSOrder(0),
            m_p1M(nullptr), m_p2M(nullptr), m_p1CS(nullptr), m_p2CS(nullptr), m_p12ACS(nullptr)
        {}

        /// Constructs an initialized context
        Moments2DContext(size_t p1Width, size_t p2Width, size_t p1MOrder, size_t p2MOrder, size_t p1CSOrder, size_t p2CSOrder, size_t p12ACSOrder):
            m_p1Width(0), m_p2Width(0), m_p1Card(0), m_p2Card(0),
            m_p1MOrder(0), m_p2MOrder(0), m_p1CSOrder(0), m_p2CSOrder(0), m_p12ACSOrder(0),
            m_p1M(nullptr), m_p2M(nullptr), m_p1CS(nullptr), m_p2CS(nullptr), m_p12ACS(nullptr)
        {

            (*this).init(p1Width, p2Width, p1MOrder, p2MOrder, p1CSOrder, p2CSOrder, p12ACSOrder);

        }

        /// Constructs an initialized context and fills it with val
        Moments2DContext(size_t p1Width, size_t p2Width, size_t p1MOrder, size_t p2MOrder, size_t p1CSOrder, size_t p2CSOrder, size_t p12ACSOrder, T val):
            m_p1Width(0), m_p2Width(0), m_p1Card(0), m_p2Card(0),
            m_p1MOrder(0), m_p2MOrder(0), m_p1CSOrder(0), m_p2CSOrder(0), m_p12ACSOrder(0),
            m_p1M(nullptr), m_p2M(nullptr), m_p1CS(nullptr), m_p2CS(nullptr), m_p12ACS(nullptr)
        {

            (*this).init(p1Width, p2Width, p1MOrder, p2MOrder, p1CSOrder, p2CSOrder, p12ACSOrder);
            (*this).fill(val);

        }

        /// Move constructor
        Moments2DContext(Moments2DContext&& other): m_p1Width(other.m_p1Width), m_p2Width(other.m_p2Width),
            m_p1Card(other.m_p1Card), m_p2Card(other.m_p2Card),
            m_p1MOrder(other.m_p1MOrder), m_p2MOrder(other.m_p2MOrder),
            m_p1CSOrder(other.m_p1CSOrder), m_p2CSOrder(other.m_p2CSOrder),
            m_p12ACSOrder(other.m_p12ACSOrder),
            m_p1M(std::move(other.m_p1M)), m_p2M(std::move(other.m_p2M)),
            m_p1CS(std::move(other.m_p1CS)), m_p2CS(std::move(other.m_p2CS)),
            m_p12ACS(std::move(other.m_p12ACS))
        {}

        /// Move assignment operator
        Moments2DContext& operator=(Moments2DContext&& other) {
            m_p1Width = other.m_p1Width;
            m_p2Width = other.m_p2Width;
            m_p1Card = other.m_p1Card;
            m_p2Card = other.m_p2Card;
            m_p1MOrder = other.m_p1MOrder;
            m_p2MOrder = other.m_p2MOrder;
            m_p1CSOrder = other.m_p1CSOrder;
            m_p2CSOrder = other.m_p2CSOrder;
            m_p12ACSOrder = other.m_p12ACSOrder;
            m_p1M = std::move(other.m_p1M);
            m_p2M = std::move(other.m_p2M);
            m_p1CS = std::move(other.m_p1CS);
            m_p2CS = std::move(other.m_p2CS);
            m_p12ACS = std::move(other.m_p12ACS);
            return (*this);
        }

        /// Empty destructor
        virtual ~Moments2DContext() {}

        virtual void init(size_t p1Width, size_t p2Width, size_t p1MOrder, size_t p2MOrder, size_t p1CSOrder, size_t p2CSOrder, size_t p12ACSOrder) {

            if(p1Width == m_p1Width && p2Width == m_p2Width && p1MOrder == m_p1MOrder && p2MOrder == m_p2MOrder && p1CSOrder == m_p1CSOrder && p2CSOrder == m_p2CSOrder && p12ACSOrder == m_p12ACSOrder)
                return; // already there, nothing to do

            try{

                m_p1Width = p1Width;
                m_p1Card = 0;
                m_p2Width = p2Width;
                m_p2Card = 0;
                m_p1MOrder = p1MOrder;
                m_p2MOrder = p2MOrder;
                m_p1CSOrder = p1CSOrder;
                m_p2CSOrder = p2CSOrder;
                m_p12ACSOrder = p12ACSOrder;

                // M
                m_p1M.reset(new Vector<T>[m_p1MOrder]);
                m_p2M.reset(new Vector<T>[m_p2MOrder]);

                for(size_t order = 0; order < m_p1MOrder; order++){
                    m_p1M[order].init(p1Width);
                }

                for(size_t order = 0; order < m_p2MOrder; order++){
                    m_p2M[order].init(p2Width);
                }


                // CS
                size_t p1CSLen = (m_p1CSOrder > 1) ? m_p1CSOrder - 1 : 0; // 1st order CS is a constant '0', dont allocate a Vector for that
                size_t p2CSLen = (m_p2CSOrder > 1) ? m_p2CSOrder - 1 : 0; // 1st order CS is a constant '0', dont allocate a Vector for that

                m_p1CS.reset(new Vector<T>[p1CSLen]);
                m_p2CS.reset(new Vector<T>[p2CSLen]);

                for(size_t order = 0; order < p1CSLen; order++){
                    m_p1CS[order].init(p1Width);
                }

                for(size_t order = 0; order < p2CSLen; order++){
                    m_p2CS[order].init(p2Width);
                }


                // ACS
                m_p12ACS.reset(new Matrix<T>[m_p12ACSOrder]);

                for(size_t order = 0; order < m_p12ACSOrder; order++){
                    m_p12ACS[order].init(p1Width, p2Width);
                }

            } catch (std::bad_alloc & e) {
                (void)e; // supress MSVC warning
                qFatal("Context memory allocation failed");
            }
        }

        virtual void init(size_t p1Width, size_t p2Width, size_t p1MOrder, size_t p2MOrder, size_t p1CSOrder, size_t p2CSOrder, size_t p12ACSOrder, T val) {
            (*this).init(p1Width, p2Width, p1MOrder, p2MOrder, p1CSOrder, p2CSOrder, p12ACSOrder);
            (*this).fill(val);
        }

        virtual void fill(T val) {

            // M
            for(size_t order = 0; order < m_p1MOrder; order++){
                m_p1M[order].fill(val);
            }

            for(size_t order = 0; order < m_p2MOrder; order++){
                m_p2M[order].fill(val);
            }

            // CS
            size_t p1CSLen = (m_p1CSOrder > 1) ? m_p1CSOrder - 1 : 0; // 1st order CS is a constant '0', dont allocate a Vector for that
            size_t p2CSLen = (m_p2CSOrder > 1) ? m_p2CSOrder - 1 : 0; // 1st order CS is a constant '0', dont allocate a Vector for that

            for(size_t order = 0; order < p1CSLen; order++){
                m_p1CS[order].fill(val);
            }

            for(size_t order = 0; order < p2CSLen; order++){
                m_p2CS[order].fill(val);
            }

            // ACS
            for(size_t order = 0; order < m_p12ACSOrder; order++){
                m_p12ACS[order].fill(val);
            }

        }

        /// Fill the whole context with zeroes
        virtual void reset() {

            (*this).fill(0);
            m_p1Card = 0;
            m_p2Card = 0;

        }

        /// Width of the first population
        virtual size_t p1Width() const { return m_p1Width; }
        /// Width of the second population
        virtual size_t p2Width() const { return m_p2Width; }

        /// Maximum order of the raw moments, 1 upto mOrder
        virtual size_t p1MOrder() const { return m_p1MOrder; }

        /// Maximum order of the raw moments, 1 upto mOrder
        virtual size_t p2MOrder() const { return m_p2MOrder; }

        /// Maximum order of the central moment sums, 2 upto csOrder
        virtual size_t p1CSOrder() const { return m_p1CSOrder; }

        /// Maximum order of the central moment sums, 2 upto csOrder
        virtual size_t p2CSOrder() const { return m_p2CSOrder; }

        /// Maximum order of the adjusted central moment sums, 1 upto acsOrder
        virtual size_t p12ACSOrder() const { return m_p12ACSOrder; }

        /// Cardinality of the first population
        virtual          size_t    & p1Card() { return m_p1Card; }
        /// Cardinality of the first population (const)
        virtual  const   size_t    & p1Card() const { return m_p1Card; }

        /// Cardinality of the second population
        virtual          size_t    & p2Card() { return m_p2Card; }
        /// Cardinality of the second population (const)
        virtual  const   size_t    & p2Card() const { return m_p2Card; }

        /// Raw moment of the first population, order 1 upto mOrder
        virtual          Vector<T> & p1M(size_t order)       { return m_p1M[order-1]; }
        /// Raw moment of the first population, order 1 upto mOrder (const)
        virtual  const   Vector<T> & p1M(size_t order) const { return m_p1M[order-1]; }

        /// Raw moment of the second population, order 1 upto mOrder
        virtual          Vector<T> & p2M(size_t order)       { return m_p2M[order-1]; }
        /// Raw moment of the second population, order 1 upto mOrder (const)
        virtual  const   Vector<T> & p2M(size_t order) const { return m_p2M[order-1]; }

        /// Central moment sum of the first population, order 2 upto csOrder
        virtual          Vector<T> & p1CS(size_t order)       { return m_p1CS[order-2]; }
        /// Central moment sum of the first population, order 2 upto csOrder (const)
        virtual  const   Vector<T> & p1CS(size_t order) const { return m_p1CS[order-2]; }

        /// Central moment sum of the second population, order 2 upto csOrder
        virtual          Vector<T> & p2CS(size_t order)       { return m_p2CS[order-2]; }
        /// Central moment sum of the second population, order 2 upto csOrder (const)
        virtual  const   Vector<T> & p2CS(size_t order) const { return m_p2CS[order-2]; }

        /// Adjusted central moment sum both populations, order 1 upto acsOrder
        virtual          Matrix<T> & p12ACS(size_t order)       { return m_p12ACS[order-1]; }
        /// Adjusted central moment sum both populations, order 1 upto acsOrder (const)
        virtual  const   Matrix<T> & p12ACS(size_t order) const { return m_p12ACS[order-1]; }


    protected:

        size_t m_p1Width;
        size_t m_p2Width;

        size_t m_p1Card;
        size_t m_p2Card;

        size_t m_p1MOrder;
        size_t m_p2MOrder;

        size_t m_p1CSOrder;
        size_t m_p2CSOrder;

        size_t m_p12ACSOrder;

        std::unique_ptr<Vector<T>[]> m_p1M;
        std::unique_ptr<Vector<T>[]> m_p2M;

        std::unique_ptr<Vector<T>[]> m_p1CS;
        std::unique_ptr<Vector<T>[]> m_p2CS;

        std::unique_ptr<Matrix<T>[]> m_p12ACS;
    };

}

#endif // TYPESMOMENT_HPP
