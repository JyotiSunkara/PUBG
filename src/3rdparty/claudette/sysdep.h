/*   ColDet - C++ 3D Collision Detection Library
 *   Copyright (C) 2000   Amir Geva
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 *
 * Any comments, questions and bug reports send to:
 *   photon@photoneffect.com
 *
 * Or visit the home page: http://photoneffect.com/coldet/
 */

#pragma once

///////////////////////////////////////////////////
// g++ compiler assumed on UNIX systems
///////////////////////////////////////////////////
#ifdef FOUG_OS_UNIX

typedef unsigned long DWORD;
DWORD GetTickCount();

///////////////////////////////////////////////////
// Windows compilers
///////////////////////////////////////////////////
#elif defined(FOUG_OS_WINDOWS)

  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>

///////////////////////////////////////////////////
// MacOS 9.0.4/MacOS X.  CodeWarrior Pro 6
// Thanks to Marco Tenuti for this addition
///////////////////////////////////////////////////
#elif defined(FOUG_OS_MAC)
   typedef unsigned long DWORD;
   #include <Events.h>
   #define GetTickCount() ::TickCount()

#else

#error No system specified (FOUG_OS_UNIX FOUG_OS_WINDOWS FOUG_OS_MAC)

#endif
