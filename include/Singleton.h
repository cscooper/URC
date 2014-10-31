/*
 *  Singleton.h - Template class for singleton objects.
 *  Copyright (C) 2012  C. S. Cooper, A. Mukunthan
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 *  Contact Details: Cooper - andor734@gmail.com
 */

#pragma once

/*
 *	Class:		 Singleton
 *	Inherits:	 None
 *	Description: Template class for singleton objects.
 */

template <typename T> class Singleton {

public:
	static T *m_pSingleton;

	Singleton() { m_pSingleton = static_cast<T*> (this); }
	virtual ~Singleton() { m_pSingleton = 0; }
	static T* GetSingleton() { return m_pSingleton; }
};

#define DECLARE_SINGLETON(x) template<> x *Singleton<x>::m_pSingleton = NULL;


#include <string>
#include <cstdarg>
#include <cstdio>

// simple exception class
class Exception {

private:
        std::string m_strException;

public:
        Exception( std::string strMessage, ... ) {

        	char buffer[200];
        	va_list v;
        	va_start(v, strMessage);
        	vsprintf( buffer, strMessage.c_str(), v );
        	va_end(v);
        	m_strException = std::string("Exception: ") + std::string(buffer);

        }


        ~Exception() { }

        std::string What() const throw() {

                return m_strException;

        }

};


#define THROW_EXCEPTION throw Exception


