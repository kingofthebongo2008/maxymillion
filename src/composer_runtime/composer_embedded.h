#pragma once

#include <type_traits>

namespace Composer
{
    namespace Bridge
    {
        //helper class to embed regular c++ objects in managed objected, required clr compiler
        template<typename t>
        ref class embedded
        {
            t* m_t;

            !embedded()
            {
                if (m_t != nullptr)
                {
                    static_assert(sizeof(t) > 0,"class must be defined");
                    delete m_t;
                }
            }

            ~embedded()
            {
                this->!embedded();
            }

        public:
            embedded( ) : m_t(new t)
            {

            }

            template <typename... args> 
            embedded( args&&... a) : m_t(new t( std::forward<a>... ) )
            {

            }

            static t* operator& (embedded% e)
            {
                return e.m_t;
            }

            t* get() 
            {
                return m_t;
            }

            static t* operator->(embedded% e) { return e.m_t; }
        };
    }

}
