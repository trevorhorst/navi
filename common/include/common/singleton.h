/** ****************************************************************************
 * @file singleton.h
 * @author Trevor Horst
 * @brief Inheritable singleton class
 *
 * Example usage of the singleton class:
 *
 * class MySingleton: public Singleton<MySingleton>
 * {
 *     // needs to be friend in order to
 *     // access the private constructor/destructor
 *     friend class Singleton<MySingleton>;
 * public:
 *     // Declare all public members here
 * private:
 *     MySingleton()
 *     {
 *         // Implement the constructor here
 *     }
 *     ~MySingleton()
 *     {
 *         // Implement the destructor here
 *     }
 * };
 * ****************************************************************************/

#ifndef SINGLETON_H
#define SINGLETON_H

#include <type_traits>

template< typename T >
class Singleton
{
protected:
    Singleton() noexcept = default;

    Singleton( const Singleton& ) = delete;

    Singleton& operator = ( const Singleton& ) = delete;

    virtual ~Singleton() = default; // to silence base class Singleton<T> has a
    // non-virtual destructor [-Weffc++]

public:
    static T& getInstance() noexcept( std::is_nothrow_constructible< T >::value )
    {
        // Guaranteed to be destroyed.
        // Instantiated on first use.
        // Thread safe in C++11
        static T instance;

        return instance;
    }
};

#endif // SINGLETON_H
